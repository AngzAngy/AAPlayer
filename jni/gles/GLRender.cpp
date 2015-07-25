#include "GLRender.h"
#include "SelfDef.h"
#include <stdlib.h>

static const GLfloat imageVertices[] = {
    -1.0f, -1.0f,
    1.0f, -1.0f,
    -1.0f,  1.0f,
    1.0f,  1.0f,
};


static const GLfloat TextureCoordinates[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
};

GLRender::GLRender():mProgram(NULL),mDataCB(NULL),mUserData(NULL){
    memset(mTextures, 0, sizeof(GLTexture2d *)*MAX_TEXTURE_NUM);
}

GLRender::~GLRender(){
    deleteC(mProgram);
    for(int i=0;i<MAX_TEXTURE_NUM;i++){
        deleteC(mTextures[i]);
    }
}

void GLRender::setRenderDataCB(RenderDataCB cb, void *userData){
    mDataCB = cb;
    mUserData = userData;
}

void GLRender::createGLProgram(const char * vertexShader, const char * fragShaderSrc){
    deleteC(mProgram);
    mProgram = new GLProgram(vertexShader, fragShaderSrc);
}

void GLRender::createTexture(const GLvoid* pixels, int w, int h, GLint format, int textureUnit){
    int index = textureUnit - GL_TEXTURE0;
    deleteC(mTextures[index]);
    mTextures[index]= new GLTexture2d(pixels, w, h, format, textureUnit, GL_UNSIGNED_BYTE);
}

void GLRender::perRender(){

}

void GLRender::postRender(){
}

void GLRender::render(){
    if(!mProgram){
        return;
    }
    GLuint program = mProgram->getProgramId();
    mProgram->useProgram();

    if(mDataCB){
        mDataCB(getImagePtr(), mUserData);
    }
    perRender();

    GLint PositionAttribute = glGetAttribLocation(program, "aPosition");
    GLint TextureCoordinateAttribute = glGetAttribLocation(program, "aTextureCoord");

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnableVertexAttribArray(PositionAttribute);
    glVertexAttribPointer(PositionAttribute, 2, GL_FLOAT, 0, 0, imageVertices);
    glEnableVertexAttribArray(TextureCoordinateAttribute);
    glVertexAttribPointer(TextureCoordinateAttribute, 2, GL_FLOAT, 0, 0, TextureCoordinates);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//    checkGlError("glDrawArrays");

    postRender();
}

