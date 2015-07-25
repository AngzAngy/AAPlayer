LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := ffmpeg/lib/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swscale
LOCAL_SRC_FILES := ffmpeg/lib/libswscale.a
LOCAL_LDLIBS := -lm
LOCAL_STATIC_LIBRARIES := avutil
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swresample
LOCAL_SRC_FILES := ffmpeg/lib/libswresample.a
LOCAL_LDLIBS := -lm
LOCAL_STATIC_LIBRARIES := avutil
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avcodec
LOCAL_SRC_FILES := ffmpeg/lib/libavcodec.a
LOCAL_LDLIBS := -lm -lz
LOCAL_STATIC_LIBRARIES := avutil swresample
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avformat
LOCAL_SRC_FILES := ffmpeg/lib/libavformat.a
LOCAL_LDLIBS := -lm -lz
LOCAL_STATIC_LIBRARIES := avutil swresample avcodec
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avfilter
LOCAL_SRC_FILES := ffmpeg/lib/libavfilter.a
LOCAL_LDLIBS := -lm -lz
LOCAL_STATIC_LIBRARIES := avutil swresample avcodec swscale avformat
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := aamediaplayer

LOCAL_CFLAGS := -D__STDC_CONSTANT_MACROS -D_ANDROID_
LOCAL_CPPFLAGS :=  -std=gnu++11

GLSRC := gles

LOCAL_C_INCLUDES := $(LOCAL_PATH)/ffmpeg/include \
                    $(LOCAL_PATH)/gles/include


LOCAL_SRC_FILES := \
    $(GLSRC)/GLProgram.cpp \
    $(GLSRC)/GLTexture2d.cpp \
    $(GLSRC)/GLRender.cpp \
    $(GLSRC)/GLYUV420PRender.cpp \
    packetqueue.cpp \
    output.cpp \
    mediaplayer.cpp \
    decoder.cpp \
    decoder_audio.cpp \
    decoder_video.cpp \
    thread.cpp \
    image-util.cpp \
    PlayAudio.cpp \
    VideoJni.cpp \
    GLVideoJni.cpp

LOCAL_LDLIBS := -llog  -landroid -lz -lGLESv2 -lOpenSLES

LOCAL_STATIC_LIBRARIES := avcodec avfilter avformat avutil swresample swscale
include $(BUILD_SHARED_LIBRARY)