#ifndef __AAIMAGE_H_
#define __AAIMAGE_H_
#include <stdint.h>

 #define        RGBA32_R8G8B8A8        0x101

 #ifdef __cplusplus
 extern "C" {
 #endif

 typedef struct __image__
 {
	 uint32_t    pixelFormat;
	 uint32_t    width;
	 uint32_t    height;
	 void*       plane[4];
	 int32_t     pitch[4];
 }Image;

 #ifdef __cplusplus
 }
 #endif

 #endif /* __AAIMAGE_H_ */
