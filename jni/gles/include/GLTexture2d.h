#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _Included_GLTexture2d
#define _Included_GLTexture2d


class GLTexture2d {

public:

    GLTexture2d(const GLvoid* pixels, int w, int h, GLint format, int texturename,GLenum type = GL_UNSIGNED_BYTE);
    GLTexture2d();

    ~GLTexture2d();

    void genTexture(const GLvoid* pixels, int w, int h, GLint format, int texturename,GLenum type = GL_UNSIGNED_BYTE);
    void bindTexture();
    void subImage(const GLvoid* pixels, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type = GL_UNSIGNED_BYTE);
    int getWidth();
    int getHeight();

    int getTextureUnit();
    GLuint getTextureId();

private:
    GLuint mTextureId;
    int mTextureUnit;
    int mWidth;
    int mHeight;
};

#endif
