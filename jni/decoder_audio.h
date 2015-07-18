#ifndef FFMPEG_DECODER_AUDIO_H
#define FFMPEG_DECODER_AUDIO_H

#include "decoder.h"

typedef void (*AudioDecodingCB) (AVFrame*,void*);

class DecoderAudio : public IDecoder
{
public:
    DecoderAudio(AVStream* stream);

    ~DecoderAudio();

    AudioDecodingCB		onDecode;
    void *userData;

private:
//    int16_t*                    mSamples;
//    int                         mSamplesSize;
    AVFrame *pFrame;

    bool                        prepare();
    bool                        decode(void* ptr);
    bool                        process(AVPacket *packet);
};

#endif //FFMPEG_DECODER_AUDIO_H
