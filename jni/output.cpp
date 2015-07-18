#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "output.h"
#include "Errors.h"

#define TAG "FFMpegOutput"

//-------------------- Audio driver --------------------

/*int Output::AudioDriver_register()
{
	return AndroidAudioTrack_register();
}

int Output::AudioDriver_unregister()
{
	return AndroidAudioTrack_unregister();
}

int Output::AudioDriver_start()
{
	return AndroidAudioTrack_start();
}

int Output::AudioDriver_set(int streamType,
							uint32_t sampleRate,
							int format,
							int channels)
{
	return AndroidAudioTrack_set(streamType,
								 sampleRate,
								 format,
								 channels);
}

int Output::AudioDriver_flush()
{
	return AndroidAudioTrack_flush();
}

int Output::AudioDriver_stop()
{
	return AndroidAudioTrack_stop();
}

int Output::AudioDriver_reload()
{
	return AndroidAudioTrack_reload();
}

int Output::AudioDriver_write(void *buffer, int buffer_size)
{
	return AndroidAudioTrack_write(buffer, buffer_size);
}*/

//-------------------- Video driver --------------------
static ANativeWindow* aNativeWin=NULL;

int Output::surface_register(JNIEnv* env, jobject jsurface){
    aNativeWin = ANativeWindow_fromSurface(env, jsurface);
    if(aNativeWin){
        return NO_ERROR;
    }else{
        return BAD_VALUE;
    }
}

int Output::surface_unregister(){
    if(aNativeWin){
        ANativeWindow_release(aNativeWin);
        aNativeWin=NULL;
    }
    return NO_ERROR;
}

int Output::surface_getWidth(){
    if(aNativeWin){
        return ANativeWindow_getWidth(aNativeWin);
    }
    return 0;
}

int Output::surface_getHeight(){
    if(aNativeWin){
        return ANativeWindow_getHeight(aNativeWin);
    }
    return 0;
}

int Output::surface_lockPixels(int *width, int *height, int *stride, void** pixels){
    if(aNativeWin && width && height && pixels){
        ANativeWindow_Buffer outBuffer;
        ANativeWindow_lock(aNativeWin, &outBuffer, NULL);
        *width=outBuffer.width;
        *height=outBuffer.height;
        if(stride){
            *stride=outBuffer.stride;
        }
        *pixels=outBuffer.bits;
        return NO_ERROR;
    }else{
        return BAD_VALUE;
    }
}

int Output::surface_unlockPixels(){
    if(aNativeWin){
        ANativeWindow_unlockAndPost(aNativeWin);
        return OK;
    }else{
        return BAD_VALUE;
    }
}
