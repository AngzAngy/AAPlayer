#include <mutex>
#include "GLRender.h"

#ifndef _Included_GLYUV420PRender
#define _Included_GLYUV420PRender

#define YUV420P_VS "attribute vec4 aPosition; \
attribute vec2 aTextureCoord; \
varying vec2 vTextureCoord; \
void main() { gl_Position = aPosition; vTextureCoord = aTextureCoord;}"

#define YUV420P_FS "precision mediump float; \
varying vec2 vTextureCoord; \
uniform sampler2D ytexture; \
uniform sampler2D utexture; \
uniform sampler2D vtexture; \
void main(void) { \
    vec3 yuv;  vec3 rgb; \
    yuv.r = texture2D(ytexture, vTextureCoord).r; \
    yuv.g = texture2D(utexture, vTextureCoord).r - 0.5; \
    yuv.b = texture2D(vtexture, vTextureCoord).r - 0.5; \
    rgb = mat3( 1,       1,         1, \
                0,       -0.39465,  2.03211, \
                1.13983, -0.58060,  0) * yuv; \
    gl_FragColor = vec4(rgb, 1); }"

//vec3 yuvDecode(vec2 texCoord) {float y = texture2D(ytexture, texCoord).r; \
//y -= 0.0627; y *= 1.164; \
//float u = texture2D(utexture, texCoord).r; \
//float v = texture2D(vtexture, texCoord).r; \
//vec2 uv = vec2(v, u); \
//uv -= 0.5; \
//vec3 rgb; \
//rgb = vec3(y); \
//rgb += vec3( 1.596 * uv.x, - 0.813 * uv.x - 0.391 * uv.y, 2.018 * uv.y); \
//return rgb; } \
//void main() { vec4 color = vec4(yuvDecode(vTextureCoord), 1.0); \
//gl_FragColor = color;}"

using namespace std;

class GLYUV420PRender: public GLRender{

public:

    GLYUV420PRender();
    virtual ~GLYUV420PRender();

    virtual void perRender();
    virtual void postRender();

    virtual void allocBuf(const int bufIdx, const uint32_t bufWidth, const uint32_t bufHeight);
    virtual void freeBuf(const int bufIdx);
	virtual bool lockBuf(const int bufIdx, Image *pImg);
    virtual bool unlockBuf(const int bufIdx);

    virtual Image *getImagePtr();
private:
	Image mImg;
    mutex mImgMutex;
};

#endif
