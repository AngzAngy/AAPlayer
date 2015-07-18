#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <stdint.h>
#include "GLProgram.h"
#include "GLTexture2d.h"
#include "Image.h"

#ifndef _Included_GLRender
#define _Included_GLRender

#define MAX_TEXTURE_NUM 3

class GLRender {

public:

    GLRender();
    virtual ~GLRender();

    void createGLProgram(const char * vertexShader, const char * fragShaderSrc);
    void createTexture(const GLvoid* pixels, int w, int h, GLint format, int textureUnit);
    virtual void perRender();
    virtual void render();
    virtual void postRender();

    virtual void allocBuf(const int bufIdx, const uint32_t bufWidth, const uint32_t bufHeight){};
    virtual void freeBuf(const int bufIdx){};
    virtual bool lockBuf(const int bufIdx, Image *pImg){return false;}
    virtual bool unlockBuf(const int bufIdx){return false;}
protected:
    GLProgram *mProgram;
    GLTexture2d *mTextures[MAX_TEXTURE_NUM];
};

#endif
