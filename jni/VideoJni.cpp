#include <stdio.h>
#include <jni.h>
#include "jnilogger.h"
#include "VideoJni.h"
#include "mediaplayer.h"

static MediaPlayer *pPlayer=NULL;
JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_init
  (JNIEnv *env, jclass clazz){
    if(pPlayer){
        delete pPlayer;
    }
    pPlayer = new MediaPlayer;
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_setSurface
  (JNIEnv *env, jclass clazz, jobject jsurface){
    if(pPlayer){
        pPlayer->setVideoSurface(env,jsurface);
    }
}


JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_surfaceDestroyed
  (JNIEnv *env, jclass clazz){
}


JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_setDataSource
  (JNIEnv *env, jclass clazz, jstring jDataSrc){
    if(!jDataSrc){
        return;
    }
    const char *fn = env->GetStringUTFChars(jDataSrc, 0);
    if(pPlayer){
        pPlayer->setDataSource(fn);
    }
    env->ReleaseStringUTFChars(jDataSrc, fn);
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_prepare
(JNIEnv *env, jclass clazz){
    if(pPlayer){
        pPlayer->prepare();
    }
}

JNIEXPORT jint JNICALL Java_com_angzangy_jni_VideoJni_getVideoWidth
  (JNIEnv *env, jclass clazz){
    int jwidth = 0;
    if(pPlayer){
        pPlayer->getVideoWidth(&jwidth);
    }
    return jwidth;
}


JNIEXPORT jint JNICALL Java_com_angzangy_jni_VideoJni_getVideoHeight
  (JNIEnv *env, jclass clazz){
    int jheight = 0;
    if(pPlayer){
        pPlayer->getVideoHeight(&jheight);
    }
    return jheight;
}

/*
 * Class:     com_angzangy_jni_VideoJni_start
 * Method:    start
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_start
  (JNIEnv *env, jclass clazz){
    if(pPlayer){
        pPlayer->start();
    }
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_pause
  (JNIEnv *env, jclass clazz){
    if(pPlayer){
        pPlayer->pause();
    }
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_stop
  (JNIEnv *env, jclass clazz){
    if(pPlayer){
        pPlayer->stop();
    }
}


JNIEXPORT void JNICALL Java_com_angzangy_jni_VideoJni_release
  (JNIEnv *env, jclass clazz){
    if(pPlayer){
        pPlayer->suspend();
        delete pPlayer;
        pPlayer=NULL;
    }
}
