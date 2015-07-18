#ifndef FFMPEG_OUTPUT_H
#define FFMPEG_OUTPUT_H

#include <jni.h>
// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

typedef void (*AudioCB)(void*);

class Output{
public:	
//	static int					AudioDriver_register();
//    static int					AudioDriver_set(int streamType,
//												uint32_t sampleRate,
//												int format,
//												int channels);
//    static int					AudioDriver_start();
//    static int					AudioDriver_flush();
//	static int					AudioDriver_stop();
//    static int					AudioDriver_reload();
//	static int					AudioDriver_write(void *buffer, int buffer_size);
//	static int					AudioDriver_unregister();
    //audio
    static void                 createAudioEngine();
    static void                 setAudioCallback(AudioCB audioCB, void *userdata);
    static void                 createBufferQueueAudioPlayer(int rate, int channel,int bitsPerSample);
    static void                 writeAudioBuf(const void*buffer, int size);
    static void                 shutdownAudio();

	static int					surface_register(JNIEnv* env, jobject jsurface);
	static int                  surface_getWidth();
	static int                  surface_getHeight();
    static int					surface_lockPixels(int *width, int *height, int *strid, void** pixels);
    static int					surface_unlockPixels();
    static int					surface_unregister();
};

#endif //FFMPEG_DECODER_H
