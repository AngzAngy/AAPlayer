#include <assert.h>
#include <string>
#include <thread>
#include "GLVideoJni.h"
#include "jnilogger.h"
#include "GLRGBARender.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/pixfmt.h"
#ifdef __cplusplus
}
#endif

#define V_Width_Name "mVideoWidth"
#define V_Height_Name "mVideoHeight"

#define Decode_Format AV_PIX_FMT_RGBA

enum {
    PLAY_STATE, PAUST_STATE, STOP_STATE
};

jobject gThiz = NULL;

AVFormatContext *formatctx = NULL;
AVCodecContext *codecctx = NULL;
int videoStream;
AVFrame *decodedFrame = NULL;

int displayWidth=0;
int displayHeight=0;
SwsContext *swsctx = NULL;

int videoState;

GLRender *gRender=NULL;
bool setDataSource(const char *fileName){
    AVCodec *pCodec = NULL;
    AVDictionary *pDict = NULL;

    AVCodec *paudioCodec = NULL;
    AVDictionary *paudioDict = NULL;

    av_register_all();

    if(avformat_open_input(&formatctx, fileName, NULL, NULL) < 0){
        return false;
    }
    if(avformat_find_stream_info(formatctx, NULL) < 0){
        return false;
    }
    av_dump_format(formatctx, 0, fileName, 0);

    videoStream = -1;
    for(int i=0;i<formatctx->nb_streams;i++){
        if(formatctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    if(-1 == videoStream){
        return false;
    }

    codecctx = formatctx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(codecctx->codec_id);
    if(NULL == pCodec) {
        return false;
    }

    if(avcodec_open2(codecctx, pCodec, &pDict) < 0){
        return false;
    }

    decodedFrame = av_frame_alloc();
    if(NULL == decodedFrame){
        return false;
    }
    return true;
}

bool setDisplaySize() {
    if(displayWidth<=0){
        displayWidth=codecctx->width;
    }
    if(displayHeight<=0){
        displayHeight=codecctx->height;
    }
    int dspW = displayWidth;
    int dspH = displayHeight;
    swsctx = sws_getContext(
            codecctx->width,
            codecctx->height,
            codecctx->pix_fmt,
            dspW,
            dspH,
            Decode_Format,
            SWS_BILINEAR,
            NULL,
            NULL,
            NULL);
    if(NULL==swsctx){
        return false;
    }

    return true;
}

void decodefn(){
    bool loop = true;
    AVPacket packet;
    AVPicture pict;
    memset(&pict, 0, sizeof(AVPicture));
    int index = 0;
    int frameFinished;
    int lineCnt;
LOGE("mylog,0000disSize(%dx%d)", displayWidth, displayHeight);
    setDisplaySize();
LOGE("mylog,1111disSize(%dx%d)", displayWidth, displayHeight);
    if(gRender==NULL){
        gRender=new GLRGBARender();
    }
    LOGE("mylog,222disSize(%dx%d)", displayWidth, displayHeight);
    gRender->allocBuf(0, displayWidth, displayHeight);
    LOGE("mylog,3333disSize(%dx%d)", displayWidth, displayHeight);
    while(loop){
        switch(videoState){
        case PLAY_STATE:
            if(av_read_frame(formatctx, &packet)>=0){
                if(packet.stream_index == videoStream){
                    avcodec_decode_video2(codecctx, decodedFrame, &frameFinished, &packet);
                    if(frameFinished){
LOGE("mylog,4444disSize(%dx%d)", displayWidth, displayHeight);
                        void *buf=NULL;
                        uint32_t w=0;
                        uint32_t h=0;
                        uint32_t stride=0;
                        gRender->lockBuf(0, &buf, &w, &h, &stride);
                        avpicture_fill(&pict, (const uint8_t *)buf, Decode_Format, displayWidth, displayHeight);
                        sws_scale(
                            swsctx,
                            (uint8_t const * const *)decodedFrame->data,
                            decodedFrame->linesize,
                            0,
                            codecctx->height,
                            pict.data,
                            pict.linesize
                        );
LOGE("mylog,5555disSize(%dx%d)", displayWidth, displayHeight);
                        gRender->unlockBuf(0);
LOGE("mylog,6666disSize(%dx%d)", displayWidth, displayHeight);
                    }
                }
                av_free_packet(&packet);
            }else{
                loop = false;
            }
            break;
        case PAUST_STATE:
//            pthread_mutex_lock(&stateMutex);
//            pthread_cond_wait(&stateCond, &stateMutex);
//            pthread_mutex_unlock(&stateMutex);
            break;
        case STOP_STATE:
            loop = false;
            break;
        }
    }

}

void myplay() {
    videoState = PLAY_STATE;

    std::thread decodeThread(decodefn);
}

void mypause() {
    videoState = PAUST_STATE;
}

void mystop() {
    videoState = STOP_STATE;
    if (videoState == PAUST_STATE) {
    }
}

void myrelease() {
    displayWidth = 0;
    displayHeight = 0;
    av_free(decodedFrame);
    avcodec_close(codecctx);
    avformat_close_input(&formatctx);
}

void jniSetVideoSize(JNIEnv *env, jobject jobj, jint w, jint h){
    jclass cls = env->GetObjectClass(jobj);
    jfieldID wfid = env->GetFieldID(cls, V_Width_Name, "I");
    jfieldID hfid = env->GetFieldID(cls, V_Height_Name, "I");
    env->SetIntField(jobj, wfid, w);
    env->SetIntField(jobj, hfid, h);
    env->DeleteLocalRef(cls);
}

JNIEXPORT void
JNICALL Java_com_angzangy_jni_GLVideoJni_nativeInit
  (JNIEnv *env, jclass jclazz, jobject obj){
    gThiz = env->NewGlobalRef(obj);
    gRender=new GLRGBARender();
}

JNIEXPORT void
JNICALL Java_com_angzangy_jni_GLVideoJni_nativeSetDataSource
  (JNIEnv *env, jclass jclazz, jstring jFileName){
    LOGI("in func %s", __FUNCTION__);
    const char *fileName = env->GetStringUTFChars(jFileName, NULL);
    if(setDataSource(fileName) && gThiz){
        jniSetVideoSize(env, gThiz, (jint)codecctx->width, (jint)codecctx->height);
    }
    LOGI("out func %s,,filename:%s", __FUNCTION__,fileName);
    env->ReleaseStringUTFChars(jFileName, fileName);
}

JNIEXPORT void
JNICALL Java_com_angzangy_jni_GLVideoJni_nativeSetDiplaySize
  (JNIEnv *env, jclass jclazz, jint jw, jint jh){
    displayWidth = jw;
    displayHeight = jh;
}

JNIEXPORT void JNICALL
Java_com_angzangy_jni_GLVideoJni_nativePlay
  (JNIEnv *env, jclass jclazz){
    LOGI("in func %s", __FUNCTION__);
    myplay();
    LOGI("out func %s", __FUNCTION__);
}

JNIEXPORT void JNICALL
Java_com_angzangy_jni_GLVideoJni_nativePause
  (JNIEnv *env, jclass jclazz){
    LOGI("in func %s", __FUNCTION__);
    mypause();
    LOGI("out func %s", __FUNCTION__);
}

JNIEXPORT void JNICALL
Java_com_angzangy_jni_GLVideoJni_nativeStop
(JNIEnv *env, jclass jclazz) {
    LOGI("in func %s", __FUNCTION__);
    mystop();
    LOGI("out func %s", __FUNCTION__);
}

JNIEXPORT void JNICALL
Java_com_angzangy_jni_GLVideoJni_nativeRelase
  (JNIEnv *env, jclass jclazz){
    myrelease();
    if(gRender){
        delete gRender;
    }
    gRender=NULL;
    if(gThiz){
        env->DeleteGlobalRef(gThiz);
    }
    gThiz=NULL;
}

JNIEXPORT void JNICALL
Java_com_angzangy_jni_GLVideoJni_nativeOnSurfaceCreated
  (JNIEnv *env, jclass jclazz){
    char vertexShader[] = "attribute vec4 aPosition;"
            "attribute vec2 aTextureCoord;"
            "varying vec2 vTextureCoord;"
            "void main() {"
            " gl_Position = aPosition; "
            "vTextureCoord = aTextureCoord;}";

    char fragShaderSrc[] = "precision mediump float;"
            "varying vec2 vTextureCoord;"
            "uniform sampler2D texture;"
            "void main() {"
            "gl_FragColor = texture2D(texture, vTextureCoord);}";

    if(gRender){
        gRender->createGLProgram(vertexShader, fragShaderSrc);
    }
}

JNIEXPORT void JNICALL
Java_com_angzangy_jni_GLVideoJni_nativeOnSurfaceChanged
  (JNIEnv *env, jclass jclazz, jint jwidth, jint jheight){
    if(gRender){
        gRender->createTexture(NULL,
                displayWidth, displayHeight, GL_RGBA, GL_TEXTURE0);
    }
}

JNIEXPORT void JNICALL
Java_com_angzangy_jni_GLVideoJni_nativeOnRender
(JNIEnv *env, jclass jclazz){
    if(gRender){
        gRender->render();
    }
}
