/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_angzangy_jni_GLVideoJni */

#ifndef _Included_com_angzangy_jni_GLVideoJni
#define _Included_com_angzangy_jni_GLVideoJni
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_nativeOnSurfaceCreated
  (JNIEnv *, jclass);

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_nativeOnSurfaceChanged
  (JNIEnv *, jclass, jint, jint);

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_nativeOnRender
  (JNIEnv *, jclass);


JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_nativeInit
  (JNIEnv *, jclass, jobject);

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_nativeSetDataSource
  (JNIEnv *, jclass, jstring);

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_nativeSetDiplaySize
  (JNIEnv *, jclass, jint, jint);

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_nativePlay
  (JNIEnv *, jclass);

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_nativePause
  (JNIEnv *, jclass);

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_nativeStop
  (JNIEnv *, jclass);

JNIEXPORT void JNICALL Java_com_angzangy_jni_GLVideoJni_nativeRelase
  (JNIEnv *, jclass);
#ifdef __cplusplus
}
#endif
#endif
