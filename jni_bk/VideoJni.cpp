#include <stdio.h>
#include <math.h>
#include <string>
#include <assert.h>
#include <pthread.h>
#include <jni.h>
#include <android/bitmap.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "jnilogger.h"
#include "VideoJni.h"
#ifdef __cplusplus
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/pixfmt.h"
#include "libavutil/time.h"
}
#endif

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000
#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)
#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0
#define SAMPLE_CORRECTION_PERCENT_MAX 10
#define AUDIO_DIFF_AVG_NB 20
#define FF_ALLOC_EVENT   (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT (SDL_USEREVENT + 2)
#define VIDEO_PICTURE_QUEUE_SIZE 1
#define DEFAULT_AV_SYNC_TYPE AV_SYNC_VIDEO_MASTER

using namespace std;

typedef struct PacketQueue {
  AVPacketList *first_pkt, *last_pkt;
  int nb_packets;
  int size;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} PacketQueue;
typedef struct VideoPicture {
//  SDL_Overlay *bmp;
  void *bmp;
  int width, height; /* source height & width */
  int allocated;
  double pts;
} VideoPicture;

typedef struct VideoState {
  AVFormatContext *pFormatCtx;
  int             videoStream, audioStream;

  int             av_sync_type;
  double          external_clock; /* external clock base */
  int64_t         external_clock_time;
  int             seek_req;
  int             seek_flags;
  int64_t         seek_pos;

  double          audio_clock;
  AVStream        *audio_st;
  PacketQueue     audioq;
  uint8_t         audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
  unsigned int    audio_buf_size;
  unsigned int    audio_buf_index;
  AVPacket        audio_pkt;
  uint8_t         *audio_pkt_data;
  int             audio_pkt_size;
  int             audio_hw_buf_size;
  double          audio_diff_cum; /* used for AV difference average computation */
  double          audio_diff_avg_coef;
  double          audio_diff_threshold;
  int             audio_diff_avg_count;
  double          frame_timer;
  double          frame_last_pts;
  double          frame_last_delay;
  double          video_clock; ///<pts of last decoded frame / predicted pts of next decoded frame
  double          video_current_pts; ///<current displayed pts (different from video_clock if frame fifos are used)
  int64_t         video_current_pts_time;  ///<time (av_gettime) at which we updated video_current_pts - used to have running video pts
  AVStream        *video_st;
  PacketQueue     videoq;
  VideoPicture    pictq[VIDEO_PICTURE_QUEUE_SIZE];
  pthread_t      parse_tid;
  pthread_t      video_tid;
  pthread_t      audio_tid;

  string filename;
  int             quit;

  AVIOContext     *io_context;
  struct SwsContext *sws_ctx;
  struct SwrContext *swr_ctx;
  ANativeWindow* aNativeWin;
} VideoState;

enum {
  AV_SYNC_AUDIO_MASTER,
  AV_SYNC_VIDEO_MASTER,
  AV_SYNC_EXTERNAL_MASTER,
};

//SDL_Surface     *screen;

VideoState      *vStat=NULL;

/* Since we only have one decoding thread, the Big Struct
   can be global in case we need it. */
VideoState *global_video_state;
AVPacket flush_pkt;

void packet_queue_init(PacketQueue *q) {
  memset(q, 0, sizeof(PacketQueue));
  pthread_mutex_init(&(q->mutex), NULL);
  pthread_cond_init(&(q->cond), NULL);
}
int packet_queue_put(PacketQueue *q, AVPacket *pkt) {

  AVPacketList *pkt1;
  if(pkt != &flush_pkt && av_dup_packet(pkt) < 0) {
    return -1;
  }
  pkt1 = (AVPacketList*)av_malloc(sizeof(AVPacketList));
  if (!pkt1)
    return -1;
  pkt1->pkt = *pkt;
  pkt1->next = NULL;

  pthread_mutex_lock(&(q->mutex));

  if (!q->last_pkt)
    q->first_pkt = pkt1;
  else
    q->last_pkt->next = pkt1;
  q->last_pkt = pkt1;
  q->nb_packets++;
  q->size += pkt1->pkt.size;
  pthread_cond_signal(&(q->cond));

  pthread_mutex_unlock(&(q->mutex));
  return 0;
}
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int debug=0)
{
  AVPacketList *pkt1;
  int ret;

  pthread_mutex_lock(&(q->mutex));

  for(;;) {

    if(global_video_state->quit) {
      ret = -1;
      break;
    }

    pkt1 = q->first_pkt;
    if (pkt1) {
      q->first_pkt = pkt1->next;
      if (!q->first_pkt)
          q->last_pkt = NULL;
      q->nb_packets--;
      q->size -= pkt1->pkt.size;
      *pkt = pkt1->pkt;
      av_free(pkt1);
      ret = 1;
      break;
    } else if (!block) {
      ret = 0;
      break;
    } else {
        pthread_cond_wait(&(q->cond), &(q->mutex));
    }
  }
  pthread_mutex_unlock(&(q->mutex));
  return ret;
}
static void packet_queue_flush(PacketQueue *q) {
  AVPacketList *pkt, *pkt1;
  pthread_mutex_lock(&(q->mutex));
  for(pkt = q->first_pkt; pkt != NULL; pkt = pkt1) {
    pkt1 = pkt->next;
    av_free_packet(&pkt->pkt);
    av_freep(&pkt);
  }
  q->last_pkt = NULL;
  q->first_pkt = NULL;
  q->nb_packets = 0;
  q->size = 0;
  pthread_mutex_unlock(&(q->mutex));
}
double get_audio_clock(VideoState *is) {
  double pts;
  int hw_buf_size, bytes_per_sec, n;

  pts = is->audio_clock; /* maintained in the audio thread */
  hw_buf_size = is->audio_buf_size - is->audio_buf_index;
  bytes_per_sec = 0;
  n = is->audio_st->codec->channels * 2;
  if(is->audio_st) {
    bytes_per_sec = is->audio_st->codec->sample_rate * n;
  }
  if(bytes_per_sec) {
    pts -= (double)hw_buf_size / bytes_per_sec;
  }
  return pts;
}
double get_video_clock(VideoState *is) {
  double delta;

  delta = (av_gettime() - is->video_current_pts_time) / 1000000.0;
  return is->video_current_pts + delta;
}
double get_external_clock(VideoState *is) {
  return av_gettime() / 1000000.0;
}
double get_master_clock(VideoState *is) {
  if(is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
    return get_video_clock(is);
  } else if(is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
    return get_audio_clock(is);
  } else {
    return get_external_clock(is);
  }
}
/* Add or subtract samples to get a better sync, return new
   audio buffer size */
int synchronize_audio(VideoState *is, short *samples,
              int samples_size, double pts) {
  int n;
  double ref_clock;

  n = 2 * is->audio_st->codec->channels;

  if(is->av_sync_type != AV_SYNC_AUDIO_MASTER) {
    double diff, avg_diff;
    int wanted_size, min_size, max_size /*, nb_samples */;

    ref_clock = get_master_clock(is);
    diff = get_audio_clock(is) - ref_clock;

    if(diff < AV_NOSYNC_THRESHOLD) {
      // accumulate the diffs
      is->audio_diff_cum = diff + is->audio_diff_avg_coef
    * is->audio_diff_cum;
      if(is->audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
    is->audio_diff_avg_count++;
      } else {
    avg_diff = is->audio_diff_cum * (1.0 - is->audio_diff_avg_coef);
    if(fabs(avg_diff) >= is->audio_diff_threshold) {
      wanted_size = samples_size + ((int)(diff * is->audio_st->codec->sample_rate) * n);
      min_size = samples_size * ((100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100);
      max_size = samples_size * ((100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100);
      if(wanted_size < min_size) {
        wanted_size = min_size;
      } else if (wanted_size > max_size) {
        wanted_size = max_size;
      }
      if(wanted_size < samples_size) {
        /* remove samples */
        samples_size = wanted_size;
      } else if(wanted_size > samples_size) {
        uint8_t *samples_end, *q;
        int nb;

        /* add samples by copying final sample*/
        nb = (samples_size - wanted_size);
        samples_end = (uint8_t *)samples + samples_size - n;
        q = samples_end + n;
        while(nb > 0) {
          memcpy(q, samples_end, n);
          q += n;
          nb -= n;
        }
        samples_size = wanted_size;
      }
    }
      }
    } else {
      /* difference is TOO big; reset diff stuff */
      is->audio_diff_avg_count = 0;
      is->audio_diff_cum = 0;
    }
  }
  return samples_size;
}

int queue_picture(VideoState *is, AVFrame *pFrame, double pts) {
  if(is->aNativeWin) {
    ANativeWindow_Buffer outBuffer;
    ANativeWindow_lock(is->aNativeWin, &outBuffer,NULL);

    //int dst_pix_fmt;
    AVPicture pict;
    pict.data[0] = (uint8_t*)outBuffer.bits;
    pict.data[1]=pict.data[2]=NULL;

    pict.linesize[0] = outBuffer.stride*4;

//    LOGE("winSize:%d x %d",
//            ANativeWindow_getWidth(is->aNativeWin),
//            ANativeWindow_getHeight(is->aNativeWin));
//    LOGE("bufferSize:%d x %d,stride: %d",
//            outBuffer.width,outBuffer.height,outBuffer.stride);
    sws_scale(is->sws_ctx,
        (uint8_t const * const *)pFrame->data,
        pFrame->linesize,
        0,
        is->video_st->codec->height,
        pict.data,
        pict.linesize
    );

    ANativeWindow_unlockAndPost(is->aNativeWin);
//    vp->pts = pts;
//
//    /* now we inform our display thread that we have a pic ready */
//    if(++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) {
//      is->pictq_windex = 0;
//    }
//    SDL_LockMutex(is->pictq_mutex);
//    is->pictq_size++;
//    SDL_UnlockMutex(is->pictq_mutex);
  }
  return 0;
}

double synchronize_video(VideoState *is, AVFrame *src_frame, double pts) {

  double frame_delay;

  if(pts != 0) {
    /* if we have pts, set video clock to it */
    is->video_clock = pts;
  } else {
    /* if we aren't given a pts, set it to the clock */
    pts = is->video_clock;
  }
  /* update the video clock */
  frame_delay = av_q2d(is->video_st->codec->time_base);
  /* if we are repeating a frame, adjust clock accordingly */
  frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
  is->video_clock += frame_delay;
  return pts;
}

uint64_t global_video_pkt_pts = AV_NOPTS_VALUE;

/* These are called whenever we allocate a frame
 * buffer. We use this to store the global_pts in
 * a frame at the time it is allocated.
 */
int our_get_buffer(struct AVCodecContext *c, AVFrame *pic, int flags) {
  int ret = avcodec_default_get_buffer2(c, pic, flags);
  uint64_t *pts = (uint64_t *)av_malloc(sizeof(uint64_t));
  *pts = global_video_pkt_pts;
  pic->opaque = pts;
  return ret;
}
void our_release_buffer(struct AVCodecContext *c, AVFrame *pic) {
  if(pic) av_freep(&pic->opaque);
//  avcodec_default_release_buffer(c, pic);
}

void* video_thread(void *arg) {
  VideoState *is = (VideoState *)arg;
  AVPacket pkt1, *packet = &pkt1;
  int frameFinished;
  AVFrame *pFrame;
  double pts;

  pFrame = av_frame_alloc();

  for(;;) {
    if(packet_queue_get(&is->videoq, packet, 1) < 0) {
      // means we quit getting packets
      break;
    }
    if(packet->data == flush_pkt.data) {
      avcodec_flush_buffers(is->video_st->codec);
      continue;
    }
    pts = 0;

    // Save global pts to be stored in pFrame in first call
    global_video_pkt_pts = packet->pts;
    // Decode video frame
    avcodec_decode_video2(is->video_st->codec, pFrame, &frameFinished,
                packet);
    if(packet->dts == AV_NOPTS_VALUE
       && pFrame->opaque && *(uint64_t*)pFrame->opaque != AV_NOPTS_VALUE) {
      pts = *(uint64_t *)pFrame->opaque;
    } else if(packet->dts != AV_NOPTS_VALUE) {
      pts = packet->dts;
    } else {
      pts = 0;
    }
    pts *= av_q2d(is->video_st->time_base);

    // Did we get a video frame?
    if(frameFinished) {
      pts = synchronize_video(is, pFrame, pts);
      if(queue_picture(is, pFrame, pts) < 0) {
    break;
      }
    }
    av_free_packet(packet);
  }
  av_free(pFrame);
  return 0;
}
void* audio_thread(void *ptr) {
  VideoState *is = (VideoState *)ptr;
  int len1, data_size = 0, n;
  AVPacket pkt1, *pkt = &pkt1;
  AVFrame *pFrame;
  double pts;

  pFrame = av_frame_alloc();

  int got_frame = 0;
  while(!(is->quit)) {
      /* next packet */
      if(packet_queue_get(&is->audioq, pkt, 1, 1) < 0) {
          continue;
      }
      if(pkt->data == flush_pkt.data) {
        avcodec_flush_buffers(is->audio_st->codec);
        continue;
      }
      len1 = avcodec_decode_audio4(is->audio_st->codec, pFrame, &got_frame, pkt);
      if(len1 < 0) {
          /* if error, skip frame */
          continue;
      }
      if (got_frame){
          data_size =av_samples_get_buffer_size(NULL,
                is->audio_st->codec->channels,
                pFrame->nb_samples,
                AV_SAMPLE_FMT_S16,
                0);
//          LOGE("audio_thread1");
          if(data_size <= 0) {
              /* No data yet, get more frames */
              continue;
          }
//          LOGE("audio_thread2");
//          if(is->swr_ctx){
//              LOGE("audio_thread ctx");
//          }else{
//              LOGE("audio_thread ctx null");
//          }
//          if(pFrame->data){
//              LOGE("audio_thread pFrame->data");
//          }else{
//              LOGE("audio_thread pFrame->data null");
//          }
//          if(pFrame){
//              LOGE("audio_thread pFrame");
//          }else{
//              LOGE("audio_thread pFrame null");
//          }
          uint8_t* tmpBuf = is->audio_buf;
//          if(tmpBuf){
//              LOGE("audio_thread tmpBuf");
//          }else{
//              LOGE("audio_thread tmpBuf null");
//          }
          int outsamples = swr_convert(is->swr_ctx,
                                      &tmpBuf,
                                      data_size,
                                      (const uint8_t**)pFrame->data,
                                      pFrame->nb_samples);
          int bytes_per_sample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
          LOGE("audio_thread: %d",bytes_per_sample);
          int resampled_data_size = outsamples * is->audio_st->codec->channels * bytes_per_sample;
          av_audioWrite((const void*)(is->audio_buf), resampled_data_size);
        }
        pts = is->audio_clock;
//        *pts_ptr = pts;
        n = 2 * is->audio_st->codec->channels;
        is->audio_clock += (double)data_size /
        (double)(n * is->audio_st->codec->sample_rate);
        if(pkt->data){
          av_free_packet(pkt);
        }
        /* if update, update the audio clock w/pts */
        if(pkt->pts != AV_NOPTS_VALUE) {
          is->audio_clock = av_q2d(is->audio_st->time_base)*pkt->pts;
        }
    }
  av_free(pFrame);
}
int stream_component_open(VideoState *is, int stream_index) {

  AVFormatContext *pFormatCtx = is->pFormatCtx;
  AVCodecContext *codecCtx = NULL;
  AVCodec *codec = NULL;
  AVDictionary *optionsDict = NULL;
//  SDL_AudioSpec wanted_spec, spec;

  if(stream_index < 0 || stream_index >= pFormatCtx->nb_streams) {
    return -1;
  }

  // Get a pointer to the codec context for the video stream
  codecCtx = pFormatCtx->streams[stream_index]->codec;

//  if(codecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
    // Set audio settings from codec info
//    wanted_spec.freq = codecCtx->sample_rate;
//    wanted_spec.format = AUDIO_S16SYS;+++++++++++++++++++++++
//    wanted_spec.channels = codecCtx->channels;
//    wanted_spec.silence = 0;
//    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
//    wanted_spec.callback = audio_callback;
//    wanted_spec.userdata = is;
//
//    if(SDL_OpenAudio(&wanted_spec, &spec) < 0) {
//      fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
//      return -1;
//    }
//    is->audio_hw_buf_size = spec.size;
//  }
  codec = avcodec_find_decoder(codecCtx->codec_id);
  if(!codec || (avcodec_open2(codecCtx, codec, &optionsDict) < 0)) {
    LOGE("Unsupported codec!\n");
    return -1;
  }

  switch(codecCtx->codec_type) {
  case AVMEDIA_TYPE_AUDIO:
    is->audio_buf_size = 0;
    is->audio_buf_index = 0;

    /* averaging filter for audio sync */
    is->audio_diff_avg_coef = exp(log(0.01 / AUDIO_DIFF_AVG_NB));
    is->audio_diff_avg_count = 0;
    /* Correct audio only if larger error than this */
    is->audio_diff_threshold = 2.0 * SDL_AUDIO_BUFFER_SIZE / codecCtx->sample_rate;

    memset(&is->audio_pkt, 0, sizeof(is->audio_pkt));

    packet_queue_init(&is->audioq);
    is->swr_ctx = swr_alloc_set_opts(NULL,
                                AV_CH_LAYOUT_STEREO,
                                AV_SAMPLE_FMT_S16,
                                44100,
                                codecCtx->channel_layout,
                                codecCtx->sample_fmt ,
                                codecCtx->sample_rate,
                                0,
                                NULL);
    swr_init(is->swr_ctx);

    av_createAudioEngine();
    LOGE("audio rate: %d, channels: %d", codecCtx->sample_rate,codecCtx->channels);
    LOGE("sampleFmt: %s", av_get_sample_fmt_name(codecCtx->sample_fmt));
    LOGE("sampleFmtIndex: %d", codecCtx->sample_fmt);
    av_createBufferQueueAudioPlayer(/*codecCtx->sample_rate*/44100,
            codecCtx->channels,  SL_PCMSAMPLEFORMAT_FIXED_16);
    pthread_create(&(is->audio_tid), NULL, audio_thread, is);
    break;
  case AVMEDIA_TYPE_VIDEO:
    is->frame_timer = (double)av_gettime() / 1000000.0;
    is->frame_last_delay = 40e-3;
    is->video_current_pts_time = av_gettime();

    packet_queue_init(&is->videoq);

    LOGE("ANativeWindowSize : %d x %d",
            ANativeWindow_getWidth(is->aNativeWin),ANativeWindow_getHeight(is->aNativeWin));
    is->sws_ctx =
        sws_getContext(
            is->video_st->codec->width,
            is->video_st->codec->height,
            is->video_st->codec->pix_fmt,
            ANativeWindow_getWidth(is->aNativeWin),
            ANativeWindow_getHeight(is->aNativeWin),
            AV_PIX_FMT_RGBA,
            SWS_BILINEAR,
            NULL,
            NULL,
            NULL
        );
    codecCtx->get_buffer2 = our_get_buffer;
//    codecCtx->release_buffer = our_release_buffer;
    pthread_create(&(is->video_tid), NULL, video_thread, is);
    break;
  default:
    break;
  }

  return 0;
}

int decode_interrupt_cb(void *opaque) {
  return (global_video_state && global_video_state->quit);
}
void* decode_thread(void *arg) {

  VideoState *is = (VideoState *)arg;
  AVPacket pkt1, *packet = &pkt1;

  if(is->videoStream < 0 || is->audioStream < 0) {
      return NULL;
  }
   stream_component_open(is, is->audioStream);
   stream_component_open(is, is->videoStream);

  for(;;) {
    if(is->quit) {
      break;
    }
    // seek stuff goes here
    if(is->seek_req) {
        int stream_index= -1;
        int64_t seek_target = is->seek_pos;

        if(is->videoStream >= 0) {
            stream_index = is->videoStream;
        }else if(is->audioStream >= 0){
            stream_index = is->audioStream;
        }

        if(stream_index>=0){
            seek_target= av_rescale_q(seek_target, AV_TIME_BASE_Q, is->pFormatCtx->streams[stream_index]->time_base);
        }
        if(av_seek_frame(is->pFormatCtx, stream_index, seek_target, is->seek_flags) < 0) {
        LOGE("%s: error while seeking", is->pFormatCtx->filename);
        } else {
            if(is->audioStream >= 0) {
                packet_queue_flush(&is->audioq);
                packet_queue_put(&is->audioq, &flush_pkt);
            }
            if(is->videoStream >= 0) {
                packet_queue_flush(&is->videoq);
                packet_queue_put(&is->videoq, &flush_pkt);
            }
      }
      is->seek_req = 0;
    }

//    if(is->audioq.size > MAX_AUDIOQ_SIZE ||
//       is->videoq.size > MAX_VIDEOQ_SIZE) {
//      continue;
//    }
    if(av_read_frame(is->pFormatCtx, packet) < 0) {
      if(is->pFormatCtx->pb->error == 0) {
          continue;
      } else {
          break;
      }
    }
    // Is this a packet from the video stream?
    if(packet->stream_index == is->videoStream) {
      packet_queue_put(&is->videoq, packet);
    } else if(packet->stream_index == is->audioStream) {
      packet_queue_put(&is->audioq, packet);
    } else {
      av_free_packet(packet);
    }
  }
  return NULL;
}

void stream_seek(VideoState *is, int64_t pos, int rel) {
  if(!is->seek_req) {
    is->seek_pos = pos;
    is->seek_flags = rel < 0 ? AVSEEK_FLAG_BACKWARD : 0;
    is->seek_req = 1;
  }
}


void av_init(){
    av_release();

    vStat = new VideoState;
    vStat->av_sync_type = DEFAULT_AV_SYNC_TYPE;

    av_init_packet(&flush_pkt);
    flush_pkt.data = (unsigned char *)"FLUSH";

    // Register all formats and codecs
    av_register_all();
}
void av_prepare(){
    VideoState *is = vStat;
    AVFormatContext *pFormatCtx = NULL;
    AVPacket pkt1, *packet = &pkt1;

    AVDictionary *io_dict = NULL;
    AVIOInterruptCB callback;

    int i;

    is->videoStream=-1;
    is->audioStream=-1;

    global_video_state = is;
    // will interrupt blocking functions if we quit!
    callback.callback = decode_interrupt_cb;
    callback.opaque = is;
    LOGI("start prepare: %s", is->filename.c_str());
    if (avio_open2(&is->io_context, is->filename.c_str(), 0, &callback, &io_dict)){
        LOGE("Unable to open I/O for %s\n", is->filename.c_str());
      return;
    }

    // Open video file
    if(avformat_open_input(&pFormatCtx, is->filename.c_str(), NULL, NULL)!=0)
      return; // Couldn't open file

    is->pFormatCtx = pFormatCtx;

    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
      return; // Couldn't find stream information

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, is->filename.c_str(), 0);

    // Find the first video stream
    for(i=0; i<pFormatCtx->nb_streams; i++) {
      if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO &&
          is->videoStream < 0) {
          is->videoStream = i;
          is->video_st = pFormatCtx->streams[i];
      }
      if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO &&
         is->audioStream < 0) {
         is->audioStream = i;
         is->audio_st = pFormatCtx->streams[i];
      }
    }
    LOGI("end prepare: %s", is->filename.c_str());
    if(is->videoStream < 0 || is->audioStream < 0) {
      LOGE("%s: could not open codecs", is->filename.c_str());
      av_release();
    }
}
void av_release(){
    if(vStat){
        av_shutdownAudio();

        swr_free(&(vStat->swr_ctx));
        delete vStat;
        vStat = NULL;
    }
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_init
  (JNIEnv *env, jclass clazz){
    av_init();
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_setSurface
  (JNIEnv *env, jclass clazz, jobject jsurface){
    Java_com_angzangy_jni_VideoJni_surfaceDestroyed(env, clazz);
    vStat->aNativeWin = ANativeWindow_fromSurface(env, jsurface);
}


JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_surfaceDestroyed
  (JNIEnv *env, jclass clazz){
    if(vStat->aNativeWin){
        ANativeWindow_release(vStat->aNativeWin);
        vStat->aNativeWin=NULL;
    }
}


JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_setDataSource
  (JNIEnv *env, jclass clazz, jstring jDataSrc){
    if(!jDataSrc){
        return;
    }
    const char *fn = env->GetStringUTFChars(jDataSrc, 0);
    vStat->filename = fn;
    env->ReleaseStringUTFChars(jDataSrc, fn);
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_prepare
(JNIEnv *env, jclass clazz){
    av_prepare();
}

JNIEXPORT jint JNICALL Java_com_angzangy_jni_VideoJni_getVideoWidth
  (JNIEnv *env, jclass clazz){
    if(vStat){
        return  vStat->video_st->codec->width;
    }else{
        return 0;
    }
}


JNIEXPORT jint JNICALL Java_com_angzangy_jni_VideoJni_getVideoHeight
  (JNIEnv *env, jclass clazz){
    if(vStat){
        return  vStat->video_st->codec->height;
    }else{
        return 0;
    }
}

/*
 * Class:     com_angzangy_jni_VideoJni_start
 * Method:    start
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_start
  (JNIEnv *env, jclass clazz){
        vStat->quit = 0;
        pthread_create(&(vStat->parse_tid), NULL, decode_thread, vStat);
}

/*
 * Class:     com_angzangy_jni_VideoJni
 * Method:    pause
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_pause
  (JNIEnv *env, jclass clazz){

}

/*
 * Class:     com_angzangy_jni_VideoJni
 * Method:    stop
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_stop
  (JNIEnv *env, jclass clazz){
    vStat->quit = 1;
}


JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_release
  (JNIEnv *env, jclass clazz){
    av_release();
}
