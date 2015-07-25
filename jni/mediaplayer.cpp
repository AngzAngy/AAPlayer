/*
 * mediaplayer.cpp
 * zhangshiyang1234
 */

//#define LOG_NDEBUG 0
#define TAG "FFMpegMediaPlayer"

//#include <sys/types.h>
//#include <sys/time.h>
//#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

extern "C" {
	
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/log.h"
	
} // end of extern C

#include <android/log.h>

#include "mediaplayer.h"
#include "output.h"
#include "jnilogger.h"
#include "image-util.h"

#define FPS_DEBUGGING true

//#define DUM_A

MediaPlayer::MediaPlayer()
{
    mListener = NULL;
    mCookie = NULL;
    mDuration = -1;
/*    mStreamType = MUSIC;*/
    mCurrentPosition = -1;
    mSeekPosition = -1;
    mCurrentState = MEDIA_PLAYER_IDLE;
    mPrepareSync = false;
    mPrepareStatus = NO_ERROR;
    mLoop = false;
    pthread_mutex_init(&mLock, NULL);
    mLeftVolume = mRightVolume = 1.0;
    mVideoWidth = mVideoHeight = 0;
    mMovieFile = NULL;
    mVideoConvertCtx = NULL;
    mAudioSwrCtx = NULL;
    mDecoderAudio = NULL;
    mVideoStreamIndex = -1;
    mAudioStreamIndex = -1;
    mRawAudioBuf = NULL;
    mAudioQueue = NULL;
    mAudioFrame = NULL;
    mVideoQueue = NULL;
    mVideoFrame = NULL;

    av_register_all();
}

MediaPlayer::~MediaPlayer()
{
	if(mListener != NULL) {
		free(mListener);
	}
}

status_t MediaPlayer::prepareAudio()
{
	__android_log_print(ANDROID_LOG_INFO, TAG, "prepareAudio");
	mAudioStreamIndex = -1;
	AVDictionary *optionsDict = NULL;
	for (int i = 0; i < mMovieFile->nb_streams; i++) {
		if (mMovieFile->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			mAudioStreamIndex = i;
			break;
		}
	}
	
	if (mAudioStreamIndex == -1) {
		return INVALID_OPERATION;
	}

	AVStream* stream = mMovieFile->streams[mAudioStreamIndex];
	// Get a pointer to the codec context for the video stream
	AVCodecContext* codec_ctx = stream->codec;
	AVCodec* codec = avcodec_find_decoder(codec_ctx->codec_id);
	if (codec == NULL) {
		return INVALID_OPERATION;
	}
	
	// Open codec
	if (avcodec_open2(codec_ctx, codec, &optionsDict) < 0) {
		return INVALID_OPERATION;
	}

	// prepare os output
/*	if (Output::AudioDriver_set(MUSIC,
								stream->codec->sample_rate,
								PCM_16_BIT,
								(stream->codec->channels == 2) ? CHANNEL_OUT_STEREO
										: CHANNEL_OUT_MONO) != ANDROID_AUDIOTRACK_RESULT_SUCCESS) {
		return INVALID_OPERATION;
	}

	if (Output::AudioDriver_start() != ANDROID_AUDIOTRACK_RESULT_SUCCESS) {
		return INVALID_OPERATION;
	}*/

	return NO_ERROR;
}

status_t MediaPlayer::prepareVideo()
{
	__android_log_print(ANDROID_LOG_INFO, TAG, "prepareVideo");
	// Find the first video stream
	mVideoStreamIndex = -1;
	AVDictionary *optionsDict = NULL;
	for (int i = 0; i < mMovieFile->nb_streams; i++) {
		if (mMovieFile->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			mVideoStreamIndex = i;
			__android_log_print(ANDROID_LOG_INFO, TAG, "prepareVideo,index: %d",mVideoStreamIndex);
			break;
		}
	}
	
	if (mVideoStreamIndex == -1) {
		return INVALID_OPERATION;
	}
	
	AVStream* stream = mMovieFile->streams[mVideoStreamIndex];
	// Get a pointer to the codec context for the video stream
	AVCodecContext* codec_ctx = stream->codec;
	AVCodec* codec = avcodec_find_decoder(codec_ctx->codec_id);
	__android_log_print(ANDROID_LOG_INFO, TAG, "prepareVideo finddecoder");
	if (codec == NULL) {
	    __android_log_print(ANDROID_LOG_ERROR, TAG, "prepareVideo finddecoder fail");
		return INVALID_OPERATION;
	}
	
	// Open codec
	if (avcodec_open2(codec_ctx, codec, &optionsDict) < 0) {
	    __android_log_print(ANDROID_LOG_INFO, TAG, "prepareVideo open fail");
		return INVALID_OPERATION;
	}
	
	mVideoWidth = codec_ctx->width;
	mVideoHeight = codec_ctx->height;
	mDuration =  mMovieFile->duration;
	
	__android_log_print(ANDROID_LOG_INFO, TAG, "prepareVideo vSize(%dx%d)",mVideoWidth,mVideoHeight);

	return NO_ERROR;
}

status_t MediaPlayer::prepare()
{
    __android_log_print(ANDROID_LOG_INFO, TAG, "prepare");
	status_t ret;
	mCurrentState = MEDIA_PLAYER_PREPARING;
	av_log_set_callback(ffmpegNotify);
	if ((ret = prepareVideo()) != NO_ERROR) {
		mCurrentState = MEDIA_PLAYER_STATE_ERROR;
		__android_log_print(ANDROID_LOG_INFO, TAG, "prepareVideo error");
		return ret;
	}
	if ((ret = prepareAudio()) != NO_ERROR) {
		mCurrentState = MEDIA_PLAYER_STATE_ERROR;
		__android_log_print(ANDROID_LOG_INFO, TAG, "prepareAudio error");
		return ret;
	}
	mCurrentState = MEDIA_PLAYER_PREPARED;
	__android_log_print(ANDROID_LOG_INFO, TAG, "prepare state :%d",mCurrentState);
	return NO_ERROR;
}

status_t MediaPlayer::setListener(MediaPlayerListener* listener)
{
    __android_log_print(ANDROID_LOG_INFO, TAG, "setListener");
    mListener = listener;
    return NO_ERROR;
}

status_t MediaPlayer::setDataSource(const char *fn)
{
    __android_log_print(ANDROID_LOG_ERROR, TAG, "setDataSource ==%s", fn);

    status_t err = BAD_VALUE;
    AVIOContext  *io_context=NULL;
    if (avio_open(&io_context, fn, 0)<0){
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to open I/O %s", fn);
      return INVALID_OPERATION;
    }

    // Open video file
    if(avformat_open_input(&mMovieFile, fn, NULL, NULL)!=0){
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to open input %s", fn);
      return INVALID_OPERATION;  // Couldn't open file
    }

    // Retrieve stream information
    if(avformat_find_stream_info(mMovieFile, NULL)<0){
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Unable to find stream %s", fn);
      return INVALID_OPERATION; // Couldn't find stream information
    }

    // Dump information about file onto standard error
    av_dump_format(mMovieFile, 0, fn, 0);

	mCurrentState = MEDIA_PLAYER_INITIALIZED;
    return NO_ERROR;
}

void* MediaPlayer::startPlayer(void* ptr)
{
    __android_log_print(ANDROID_LOG_INFO, TAG, "main player thread");
    MediaPlayer *pPlayer = (MediaPlayer*)ptr;
    pPlayer->decodeMovie(ptr);
}
status_t MediaPlayer::start()
{
    if (mCurrentState != MEDIA_PLAYER_PREPARED) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "starting player thread error state :%d",mCurrentState);
        return INVALID_OPERATION;
    }
    pthread_create(&mPlayerThread, NULL, startPlayer, this);
    return NO_ERROR;
}

status_t MediaPlayer::suspend() {
	__android_log_print(ANDROID_LOG_INFO, TAG, "suspend");
	
	mCurrentState = MEDIA_PLAYER_STOPPED;
	if(mDecoderAudio != NULL) {
		mDecoderAudio->stop();
	}
	if(mDecoderVideo != NULL) {
		mDecoderVideo->stop();
	}
	
	if(pthread_join(mPlayerThread, NULL) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Couldn't cancel player thread");
	}
	
	// Close the codec
    if(mDecoderAudio != NULL) {
        delete mDecoderAudio;
    }
    if(mDecoderVideo != NULL) {
        delete mDecoderVideo;
    }
	
	// Close the video file
    avformat_close_input(&mMovieFile);

	//close OS drivers
	/*Output::AudioDriver_unregister();*/
	Output::surface_unregister();

	__android_log_print(ANDROID_LOG_ERROR, TAG, "suspended");

    return NO_ERROR;
}

/*status_t MediaPlayer::resume() {
	//pthread_mutex_lock(&mLock);
	mCurrentState = MEDIA_PLAYER_STARTED;
	//pthread_mutex_unlock(&mLock);
    return NO_ERROR;
}*/

status_t MediaPlayer::setVideoSurface(JNIEnv* env, jobject jsurface)
{ 
	if(Output::surface_register(env, jsurface) != NO_ERROR) {
		return INVALID_OPERATION;
	}
    return NO_ERROR;
}

bool MediaPlayer::shouldCancel(PacketQueue* queue)
{
	return (mCurrentState == MEDIA_PLAYER_STATE_ERROR || mCurrentState == MEDIA_PLAYER_STOPPED ||
			 ((mCurrentState == MEDIA_PLAYER_DECODED || mCurrentState == MEDIA_PLAYER_STARTED) 
			  && queue->size() == 0));
}

void MediaPlayer::renderVideoCB(Image *pImg, void *userData){
    MediaPlayer *player = (MediaPlayer *)userData;
    player->renderVideo(pImg);

      if(FPS_DEBUGGING) {
          timeval pTime;
          static int frames = 0;
          static double t1 = -1;
          static double t2 = -1;

          gettimeofday(&pTime, NULL);
          t2 = pTime.tv_sec + (pTime.tv_usec / 1000000.0);
          if (t1 == -1 || t2 > t1 + 1) {
              __android_log_print(ANDROID_LOG_INFO, "fffps", "Video fps:%i", frames);
              //sPlayer->notify(MEDIA_INFO_FRAMERATE_VIDEO, frames, -1);
              t1 = t2;
              frames = 0;
          }
          frames++;
      }
}

void MediaPlayer::renderVideo(Image *pImg){
    AVPicture pict;
    AVStream* stream = NULL;
    int pts = 0;
    int got_frame = 0;
    if(mVideoQueue && pImg){
        stream = mMovieFile->streams[mVideoStreamIndex];
        if(mVideoConvertCtx == NULL){
            mVideoConvertCtx = sws_getContext(stream->codec->width,
                    stream->codec->height,
                    stream->codec->pix_fmt,
                    pImg->width,
                    pImg->height,
                    AV_PIX_FMT_YUV420P,
                    SWS_POINT,
                    NULL,
                    NULL,
                    NULL);
        }
        if(mVideoQueue->get(&mVideoPacket, true) < 0){
            return;
        }
        avcodec_decode_video2(stream->codec, mVideoFrame, &got_frame, &mVideoPacket);
        if (got_frame) {
//            __android_log_print(ANDROID_LOG_INFO, TAG, "myimg videosize: %dx%d",stream->codec->width,stream->codec->height);
//            __android_log_print(ANDROID_LOG_INFO, TAG, "myimg size: %dx%d",pImg->width,pImg->height);
//            for(int i=0;i<3;i++){
//                __android_log_print(ANDROID_LOG_INFO, TAG, "myimg pitch[%d]: %d",i,pImg->pitch[i]);
//                __android_log_print(ANDROID_LOG_INFO, TAG, "myimg addr[%d]: %d",i,pImg->plane[i]);
//                __android_log_print(ANDROID_LOG_INFO, TAG, "myimg srcPitch[%d]: %d",i,mVideoFrame->linesize[i]);
//            }
            pict.data[0] = (uint8_t*)(pImg->plane[0]);
            pict.data[1] = (uint8_t*)(pImg->plane[1]);
            pict.data[2] = (uint8_t*)(pImg->plane[2]);
            pict.linesize[0] = pImg->pitch[0];
            pict.linesize[1] = pImg->pitch[1];
            pict.linesize[2] = pImg->pitch[2];
            sws_scale(mVideoConvertCtx,
                      mVideoFrame->data,
                      mVideoFrame->linesize,
                      0,
                      stream->codec->height,
                      pict.data,
                      pict.linesize);
#ifdef DUM_A
            static int idx = 0;
            if(idx<10){
                char fn[256]={0,};
                int bufsize = (pImg->width * pImg->height) * 3 / 2;
                sprintf(fn,"/sdcard/DCIM/my_%d_%d_%d_dump.yuv",pImg->width,pImg->height,idx);
                imageUtil::saveBufToFile((char*)(pImg->plane[0]),bufsize, fn);
                idx++;
            }
#endif//DUM_A
//            int frameSize = stream->codec->width*stream->codec->height;
//            memcpy(pImg->plane[0], mVideoFrame->data[0], frameSize);
//            memcpy(pImg->plane[1], mVideoFrame->data[1], frameSize/4);
//            memcpy(pImg->plane[2], mVideoFrame->data[2], frameSize/4);
        }
        av_free_packet(&mVideoPacket);
    }
}
//void MediaPlayer::decodeVideoCB(AVFrame* frame, double pts, void* userdata){
//    MediaPlayer *player = (MediaPlayer *)userdata;
//    player->showVideo(frame);
//
//	if(FPS_DEBUGGING) {
//		timeval pTime;
//		static int frames = 0;
//		static double t1 = -1;
//		static double t2 = -1;
//
//		gettimeofday(&pTime, NULL);
//		t2 = pTime.tv_sec + (pTime.tv_usec / 1000000.0);
//		if (t1 == -1 || t2 > t1 + 1) {
//			__android_log_print(ANDROID_LOG_INFO, "fffps", "Video fps:%i", frames);
//			//sPlayer->notify(MEDIA_INFO_FRAMERATE_VIDEO, frames, -1);
//			t1 = t2;
//			frames = 0;
//		}
//		frames++;
//	}
//
//	/*Output::VideoDriver_updateSurface();*/
//}

void MediaPlayer::showVideo(AVFrame* frame){
    if(mVideoConvertCtx == NULL){
        AVStream* stream = mMovieFile->streams[mVideoStreamIndex];
        mVideoConvertCtx = sws_getContext(stream->codec->width,
                stream->codec->height,
                stream->codec->pix_fmt,
                Output::surface_getWidth(),
                Output::surface_getHeight(),
                PIX_FMT_RGBA,
                SWS_POINT,
                NULL,
                NULL,
                NULL);
    }
    if (mVideoConvertCtx == NULL) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "mVideoConvertCtx NULL");
        return;
    }
    int width,height, stride;
    void *pixels;
    AVPicture pict;

    Output::surface_lockPixels(&width, &height, &stride, &pixels);
    __android_log_print(ANDROID_LOG_ERROR, "showVideo", "surface(%dx%d,strid: %d)", width,height,stride);
    pict.data[0] = (uint8_t*)pixels;
    pict.linesize[0] = stride*4;
    // Convert the image from its native format to RGBA
    sws_scale(mVideoConvertCtx,
              frame->data,
              frame->linesize,
              0,
              mVideoHeight,
              pict.data,
              pict.linesize);

    Output::surface_unlockPixels();
}

//void MediaPlayer::decodeAudioCB(AVFrame* frame, void* userdata){
//    MediaPlayer *player = (MediaPlayer*)userdata;
//    player->playAudio(frame);
//
//	if(FPS_DEBUGGING) {
//		timeval pTime;
//		static int frames = 0;
//		static double t1 = -1;
//		static double t2 = -1;
//
//		gettimeofday(&pTime, NULL);
//		t2 = pTime.tv_sec + (pTime.tv_usec / 1000000.0);
//		if (t1 == -1 || t2 > t1 + 1) {
//			__android_log_print(ANDROID_LOG_INFO, "fffps", "Audio fps:%i", frames);
//			//sPlayer->notify(MEDIA_INFO_FRAMERATE_AUDIO, frames, -1);
//			t1 = t2;
//			frames = 0;
//		}
//		frames++;
//	}
//
///*	if(Output::AudioDriver_write(buffer, buffer_size) <= 0) {
//		__android_log_print(ANDROID_LOG_ERROR, TAG, "Couldn't write samples to audio track");
//	}*/
//}

void MediaPlayer::decodeAudio2CB(void* userdata){
    MediaPlayer *player = (MediaPlayer*)userdata;
    player->playAudio2();

    if(FPS_DEBUGGING) {
        timeval pTime;
        static int frames = 0;
        static double t1 = -1;
        static double t2 = -1;

        gettimeofday(&pTime, NULL);
        t2 = pTime.tv_sec + (pTime.tv_usec / 1000000.0);
        if (t1 == -1 || t2 > t1 + 1) {
            __android_log_print(ANDROID_LOG_INFO, "fffps", "Audio fps:%i", frames);
            //sPlayer->notify(MEDIA_INFO_FRAMERATE_AUDIO, frames, -1);
            t1 = t2;
            frames = 0;
        }
        frames++;
    }
}
void MediaPlayer::playAudio2(){
    int got_frame = 0;
    int data_size = 0;
    AVStream* stream = mMovieFile->streams[mAudioStreamIndex];
    AVCodecContext* codec_ctx = stream->codec;
    if(mAudioSwrCtx==NULL){
        mAudioSwrCtx = swr_alloc_set_opts(NULL,
            codec_ctx->channel_layout, AV_SAMPLE_FMT_S16, 44100,

            codec_ctx->channel_layout,
            codec_ctx->sample_fmt ,
            codec_ctx->sample_rate,
            0, NULL);
        swr_init(mAudioSwrCtx);
    }
    if(mAudioSwrCtx==NULL){
        return;
    }
    if(!mAudioQueue->get(&mAudioPacket, true)){
        return;
    }

    avcodec_decode_audio4(codec_ctx, mAudioFrame, &got_frame, &mAudioPacket);
    if(!got_frame){
        return;
    }
    data_size = av_samples_get_buffer_size(NULL,
            codec_ctx->channels,
            mAudioFrame->nb_samples,
            AV_SAMPLE_FMT_S16, 0);
     if (data_size > 0) {
         if(mRawAudioBuf==NULL){
             mRawAudioBuf = new uint8_t[data_size];
         }
         swr_convert(mAudioSwrCtx,
                 &mRawAudioBuf, mAudioFrame->nb_samples,
                 (const uint8_t**)mAudioFrame->data, mAudioFrame->nb_samples);

         Output::writeAudioBuf(mRawAudioBuf, data_size);
     }

    av_free_packet(&mAudioPacket);
}

//void MediaPlayer::playAudio(AVFrame* frame){
//    AVStream* stream = mMovieFile->streams[mAudioStreamIndex];
//    AVCodecContext* codec_ctx = stream->codec;
//    if(mAudioSwrCtx==NULL){
//        mAudioSwrCtx = swr_alloc_set_opts(NULL,
//            codec_ctx->channel_layout, AV_SAMPLE_FMT_S16, 44100,
//
//            codec_ctx->channel_layout,
//            codec_ctx->sample_fmt ,
//            codec_ctx->sample_rate,
//            0, NULL);
//        swr_init(mAudioSwrCtx);
//    }
//    if(mAudioSwrCtx==NULL){
//        Output::shutdownAudio();
//        return;
//    }
//    int data_size = av_samples_get_buffer_size(NULL,
//            codec_ctx->channels,
//            frame->nb_samples,
//            AV_SAMPLE_FMT_S16, 0);
//     if (data_size > 0) {
//         if(mRawAudioBuf==NULL){
//             mRawAudioBuf = new uint8_t[data_size];
//         }
//         swr_convert(mAudioSwrCtx,
//                 &mRawAudioBuf, frame->nb_samples,
//                 (const uint8_t**)frame->data, frame->nb_samples);
//         Output::writeAudioBuf(mRawAudioBuf, data_size);
//#ifdef DUM_A
//         const char *sampfmt = av_get_sample_fmt_name(AV_SAMPLE_FMT_S16);
//         char fn[256]={0,};
//         sprintf(fn,"/sdcard/DCIM/my_rate%d_ch%d_%s.pcm",44100,
//                 codec_ctx->channels,sampfmt);
//         adump2File(mRawAudioBuf, data_size, fn);
//#endif//DUM_A
//     }
//}

void MediaPlayer::decodeMovie(void* ptr)
{
	AVPacket pPacket;
	
	AVStream* stream_audio = mMovieFile->streams[mAudioStreamIndex];
//	mDecoderAudio = new DecoderAudio(stream_audio);
//	mDecoderAudio->onDecode = decodeAudioCB;
//	mDecoderAudio->userData=this;
//	mDecoderAudio->startAsync();
	mAudioFrame = av_frame_alloc();
	mAudioQueue = new PacketQueue();
    Output::createAudioEngine();
    Output::setAudioCallback(decodeAudio2CB, this);
    Output::createBufferQueueAudioPlayer(44100, stream_audio->codec->channels, SL_PCMSAMPLEFORMAT_FIXED_16);

    mVideoFrame = av_frame_alloc();
    mVideoQueue = new PacketQueue();
	
//	AVStream* stream_video = mMovieFile->streams[mVideoStreamIndex];
//	mDecoderVideo = new DecoderVideo(stream_video);
//	mDecoderVideo->onDecode = decodeVideoCB;
//	mDecoderVideo->userData = this;
//	mDecoderVideo->startAsync();
	
	mCurrentState = MEDIA_PLAYER_STARTED;
	__android_log_print(ANDROID_LOG_ERROR, TAG, "playing %ix%i", mVideoWidth, mVideoHeight);
	while (mCurrentState != MEDIA_PLAYER_DECODED && mCurrentState != MEDIA_PLAYER_STOPPED &&
		   mCurrentState != MEDIA_PLAYER_STATE_ERROR)
	{
//		if (mDecoderVideo->packets() > FFMPEG_PLAYER_MAX_QUEUE_SIZE &&
//				mDecoderAudio->packets() > FFMPEG_PLAYER_MAX_QUEUE_SIZE) {
//			usleep(200);
//			continue;
//		}
		
		if(av_read_frame(mMovieFile, &pPacket) < 0) {
//			mCurrentState = MEDIA_PLAYER_DECODED;
			continue;
		}
		// Is this a packet from the video stream?
		if (pPacket.stream_index == mVideoStreamIndex) {
//		    __android_log_print(ANDROID_LOG_ERROR, TAG, "decode %ix%i", mVideoWidth, mVideoHeight);
//			mDecoderVideo->enqueue(&pPacket);
		    if(mVideoQueue){
		        mVideoQueue->put(&pPacket);
		    }
		} 
		else if (pPacket.stream_index == mAudioStreamIndex) {
//			mDecoderAudio->enqueue(&pPacket);
		    if(mAudioQueue){
		        mAudioQueue->put(&pPacket);
		    }
		}
		else {
			// Free the packet that was allocated by av_read_frame
			av_free_packet(&pPacket);
		}
	}
	
	//waits on end of video thread
	__android_log_print(ANDROID_LOG_ERROR, TAG, "waiting on video thread");
	int ret = -1;
	if((ret = mDecoderVideo->wait()) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Couldn't cancel video thread: %i", ret);
	}
	
	__android_log_print(ANDROID_LOG_ERROR, TAG, "waiting on audio thread");
	if((ret = mDecoderAudio->wait()) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Couldn't cancel audio thread: %i", ret);
	}
    
	Output::shutdownAudio();
	if(mAudioSwrCtx){
	    swr_free(&mAudioSwrCtx);
	    mAudioSwrCtx=NULL;
	}
	if(mRawAudioBuf){
	    delete []mRawAudioBuf;
	    mRawAudioBuf=NULL;
	}
	if(mAudioQueue){
	    delete mAudioQueue;
	    mAudioQueue=NULL;
	}
	if(mAudioFrame){
	    av_free(mAudioFrame);
	    mAudioFrame = NULL;
	}
    if(mVideoQueue){
        delete mVideoQueue;
        mVideoQueue=NULL;
    }
    if(mVideoFrame){
        av_free(mVideoFrame);
        mVideoFrame = NULL;
    }
	if(mCurrentState == MEDIA_PLAYER_STATE_ERROR) {
		__android_log_print(ANDROID_LOG_INFO, TAG, "playing err");
	}
	mCurrentState = MEDIA_PLAYER_PLAYBACK_COMPLETE;
	__android_log_print(ANDROID_LOG_INFO, TAG, "end of playing");
}

status_t MediaPlayer::stop()
{
	//pthread_mutex_lock(&mLock);
	mCurrentState = MEDIA_PLAYER_STOPPED;
	//pthread_mutex_unlock(&mLock);
    return NO_ERROR;
}

status_t MediaPlayer::pause()
{
	//pthread_mutex_lock(&mLock);
	mCurrentState = MEDIA_PLAYER_PAUSED;
	//pthread_mutex_unlock(&mLock);
	return NO_ERROR;
}

bool MediaPlayer::isPlaying()
{
    return mCurrentState == MEDIA_PLAYER_STARTED || 
		mCurrentState == MEDIA_PLAYER_DECODED;
}

status_t MediaPlayer::getVideoWidth(int *w)
{
	if (mCurrentState < MEDIA_PLAYER_PREPARED) {
		return INVALID_OPERATION;
	}
	*w = mVideoWidth;
    return NO_ERROR;
}

status_t MediaPlayer::getVideoHeight(int *h)
{
	if (mCurrentState < MEDIA_PLAYER_PREPARED) {
		return INVALID_OPERATION;
	}
	*h = mVideoHeight;
    return NO_ERROR;
}

status_t MediaPlayer::getCurrentPosition(int *msec)
{
	if (mCurrentState < MEDIA_PLAYER_PREPARED) {
		return INVALID_OPERATION;
	}
	*msec = 0/*av_gettime()*/;
	//__android_log_print(ANDROID_LOG_INFO, TAG, "position %i", *msec);
	return NO_ERROR;
}

status_t MediaPlayer::getDuration(int *msec)
{
	if (mCurrentState < MEDIA_PLAYER_PREPARED) {
		return INVALID_OPERATION;
	}
	*msec = mDuration / 1000;
    return NO_ERROR;
}

status_t MediaPlayer::seekTo(int msec)
{
    return INVALID_OPERATION;
}

status_t MediaPlayer::reset()
{
    return INVALID_OPERATION;
}

status_t MediaPlayer::setAudioStreamType(int type)
{
	return NO_ERROR;
}

void MediaPlayer::ffmpegNotify(void* ptr, int level, const char* fmt, va_list vl) {
	
	switch(level) {
			/**
			 * Something went really wrong and we will crash now.
			 */
		case AV_LOG_PANIC:
			__android_log_print(ANDROID_LOG_ERROR, TAG, "AV_LOG_PANIC: %s", fmt);
			//sPlayer->mCurrentState = MEDIA_PLAYER_STATE_ERROR;
			break;
			
			/**
			 * Something went wrong and recovery is not possible.
			 * For example, no header was found for a format which depends
			 * on headers or an illegal combination of parameters is used.
			 */
		case AV_LOG_FATAL:
			__android_log_print(ANDROID_LOG_ERROR, TAG, "AV_LOG_FATAL: %s", fmt);
			//sPlayer->mCurrentState = MEDIA_PLAYER_STATE_ERROR;
			break;
			
			/**
			 * Something went wrong and cannot losslessly be recovered.
			 * However, not all future data is affected.
			 */
		case AV_LOG_ERROR:
			__android_log_print(ANDROID_LOG_ERROR, TAG, "AV_LOG_ERROR: %s", fmt);
			//sPlayer->mCurrentState = MEDIA_PLAYER_STATE_ERROR;
			break;
			
			/**
			 * Something somehow does not look correct. This may or may not
			 * lead to problems. An example would be the use of '-vstrict -2'.
			 */
		case AV_LOG_WARNING:
			__android_log_print(ANDROID_LOG_ERROR, TAG, "AV_LOG_WARNING: %s", fmt);
			break;
			
		case AV_LOG_INFO:
			__android_log_print(ANDROID_LOG_INFO, TAG, "%s", fmt);
			break;
			
		case AV_LOG_DEBUG:
			__android_log_print(ANDROID_LOG_DEBUG, TAG, "%s", fmt);
			break;
			
	}
}

void MediaPlayer::notify(int msg, int ext1, int ext2)
{
    //__android_log_print(ANDROID_LOG_INFO, TAG, "message received msg=%d, ext1=%d, ext2=%d", msg, ext1, ext2);
    bool send = true;
    bool locked = false;

    if ((mListener != 0) && send) {
       //__android_log_print(ANDROID_LOG_INFO, TAG, "callback application");
       mListener->notify(msg, ext1, ext2);
       //__android_log_print(ANDROID_LOG_INFO, TAG, "back from callback");
	}
}
