#include <stdio.h>
#include <jni.h>
#include "jnilogger.h"
#include "GLVideoJni.h"
#include "mediaplayer.h"
#include "GLYUV420PRender.h"
#include "SelfDef.h"

static MediaPlayer *pPlayer=NULL;
static GLYUV420PRender *p420Render;

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_init
  (JNIEnv *env, jclass clazz){
    deleteC(pPlayer);
    deleteC(p420Render);
    pPlayer = new MediaPlayer;
    p420Render = new GLYUV420PRender();
}


JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_surfaceCreated
(JNIEnv *env, jclass clazz){
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_surfaceChanged
  (JNIEnv *env, jclass clazz, jint jsurfaceWidth, jint jsurfaceHeight){
    if(p420Render && pPlayer){
        jsurfaceWidth = 1280;
        jsurfaceHeight = 720;
        p420Render->allocBuf(0, jsurfaceWidth, jsurfaceHeight);
        p420Render->createGLProgram(YUV420P_VS, YUV420P_FS);
        Image *img = p420Render->getImagePtr();
        p420Render->createTexture(NULL, jsurfaceWidth, jsurfaceHeight, GL_LUMINANCE, GL_TEXTURE0);
        p420Render->createTexture(NULL, jsurfaceWidth/2, jsurfaceHeight/2, GL_LUMINANCE, GL_TEXTURE1);
        p420Render->createTexture(NULL, jsurfaceWidth/2, jsurfaceHeight/2, GL_LUMINANCE, GL_TEXTURE2);

        p420Render->setRenderDataCB(MediaPlayer::renderVideoCB, pPlayer);

        __android_log_print(ANDROID_LOG_INFO, "surfaceChanged", "myimgJni size: %dx%d",img->width,img->height);
        for(int i=0;i<3;i++){
            __android_log_print(ANDROID_LOG_INFO, "surfaceChanged", "myimgJni pitch[%d]: %d",i,img->pitch[i]);
        }
    }
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_surfaceDestroyed
  (JNIEnv *env, jclass clazz){
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_render
(JNIEnv *env, jclass clazz){
    if(p420Render){
        p420Render->render();
    }
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_setDataSource
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

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_prepare
(JNIEnv *env, jclass clazz){
    if(pPlayer){
        pPlayer->prepare();
    }
}

JNIEXPORT jint JNICALL Java_com_angzangy_jni_GLVideoJni_getVideoWidth
  (JNIEnv *env, jclass clazz){
    int jwidth = 0;
    if(pPlayer){
        pPlayer->getVideoWidth(&jwidth);
    }
    return jwidth;
}


JNIEXPORT jint JNICALL Java_com_angzangy_jni_GLVideoJni_getVideoHeight
  (JNIEnv *env, jclass clazz){
    int jheight = 0;
    if(pPlayer){
        pPlayer->getVideoHeight(&jheight);
    }
    return jheight;
}


JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_start
  (JNIEnv *env, jclass clazz){
    if(pPlayer){
        pPlayer->start();
    }
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_pause
  (JNIEnv *env, jclass clazz){
    if(pPlayer){
        pPlayer->pause();
    }
}

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_stop
  (JNIEnv *env, jclass clazz){
    if(pPlayer){
        pPlayer->stop();
    }
}


JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_release
  (JNIEnv *env, jclass clazz){
    if(pPlayer){
        pPlayer->suspend();
    }
    deleteC(pPlayer);
    deleteC(p420Render);
}
