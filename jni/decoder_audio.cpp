#include <android/log.h>
#include "decoder_audio.h"

#define TAG "FFMpegAudioDecoder"

DecoderAudio::DecoderAudio(AVStream* stream) : IDecoder(stream),pFrame(NULL),
userData(NULL)
{
}

DecoderAudio::~DecoderAudio()
{
}

bool DecoderAudio::prepare()
{
//    mSamplesSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
//    mSamples = (int16_t *) av_malloc(mSamplesSize);
//    if(mSamples == NULL) {
//    	return false;
//    }
    pFrame = av_frame_alloc();
    if(pFrame == NULL) {
        return false;
    }
    return true;
}

bool DecoderAudio::process(AVPacket *packet)
{
//    int size = mSamplesSize;
//    int len = avcodec_decode_audio3(mStream->codec, mSamples, &size, packet);

    __android_log_print(ANDROID_LOG_INFO, TAG, "decoding audio");
    int got_frame = 0;
    int len = avcodec_decode_audio4(mStream->codec, pFrame, &got_frame, packet);
    if(got_frame){
        //call handler for posting buffer to os audio driver
        onDecode(pFrame, userData);
    }
    return true;
}

bool DecoderAudio::decode(void* ptr)
{
    AVPacket        pPacket;

    __android_log_print(ANDROID_LOG_INFO, TAG, "decoding audio start");

    while(mRunning)
    {
        if(mQueue->get(&pPacket, true) < 0)
        {
            mRunning = false;
            return false;
        }
        if(!process(&pPacket))
        {
            mRunning = false;
            return false;
        }
        // Free the packet that was allocated by av_read_frame
        av_free_packet(&pPacket);
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "decoding audio ended");

    // Free audio samples buffer
    av_free(pFrame);
    return true;
}
