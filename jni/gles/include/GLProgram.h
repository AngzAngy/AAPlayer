#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _Included_GLProgram
#define _Included_GLProgram

class GLProgram {
    public:
    static GLuint createProgram ( const char * vertShaderSrc, const char * fragShaderSrc );
    static GLuint loadShader ( GLenum type, const char *shaderSrc );
public:
    GLProgram(const char * vertexShader, const char * fragShaderSrc);

    virtual ~GLProgram();

    GLuint getProgramId();
//    int dowork(texture_2d * pDsttexture, TByte* pOutData); // if pOutData != null, readpixel to pOutData
//    int save(const char ** name);
    void useProgram();
    void bindTexture(const char *name, GLuint texId, GLuint textureUnit);
    void unbindTexture(GLuint textureUnit);
    bool hasUniform(const char * name );
    void setUniformv(char* name, float value[], int length);
    void setUniform1i( const char * name, int value );
    void setUniform1f( const char * name, float value );
    void setUniform2f( const char * name, float x, float y );
    void setUniform3f( const char * name, float x, float y, float z );

protected:
    GLuint mProgramId;

    bool mUsed;
};
#endif
