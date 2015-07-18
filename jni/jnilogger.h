#ifndef __JNILOGGER_H_
#define __JNILOGGER_H_
#include<stdio.h>
#ifdef _ANDROID_
#include <android/log.h>
#endif

 #ifdef __cplusplus
 extern "C" {
 #endif

 #ifndef LOG_TAG
 #define LOG_TAG    "MY_LOG_TAG"
 #endif

#ifdef _ANDROID_
 #define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
 #define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
 #define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
 #define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
 #define LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
#endif

#ifdef WIN32
#define LOGD(var) printf((var))
#define LOGI(var) printf((var))
#define LOGW(var) printf((var))
#define LOGE(var) printf((var))
#define LOGV(var)  printf((var))
#endif

 #ifdef __cplusplus
 }
 #endif

 #endif /* __JNILOGGER_H_ */
