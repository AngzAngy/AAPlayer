#include "GLProgram.h"
#include "jnilogger.h"

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}
static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGE("err after %s \n", op);
    }
}

GLuint GLProgram::loadShader ( GLenum type, const char *shaderSrc )
{
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader ( type );
    if ( shader == 0 )
        return 0;

    // Load the shader source
    glShaderSource ( shader, 1, &shaderSrc, NULL );

    // Compile the shader
    glCompileShader ( shader );

    // Check the compile status
    glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

    glReleaseShaderCompiler();

    if ( !compiled )
    {
        GLint infoLen = 0;

        glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

        if ( infoLen > 1 )
        {
            char* infoLog = (char*)malloc (sizeof(char) * infoLen );

            glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
            LOGE ( "Error compiling shader:\n%s\n", infoLog );

            free ( infoLog );
        }

        glDeleteShader ( shader );
        return 0;
    }

    return shader;

}


GLuint GLProgram::createProgram ( const char * vertShaderSrc, const char * fragShaderSrc )
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

//    LOGE ( "vertexShader == %s ", vertShaderSrc );
    // Load the vertex/fragment shaders
    vertexShader = loadShader ( GL_VERTEX_SHADER, vertShaderSrc );
    if ( vertexShader == 0 ){
        LOGE ( "error: vertexShader == 0 " );
        return 0;
    }

//    LOGE ( "fragmentShader == %s ", fragShaderSrc );
    fragmentShader = loadShader ( GL_FRAGMENT_SHADER, fragShaderSrc );
    if ( fragmentShader == 0 )
    {
        glDeleteShader( vertexShader );
        LOGE ( "error: fragmentShader == 0 " );
        return 0;
    }

    // Create the GLProgram object
    programObject = glCreateProgram ( );

    if ( programObject == 0 )
        return 0;

    glAttachShader ( programObject, vertexShader );
    checkGlError("glAttachVertexShader");
    glAttachShader ( programObject, fragmentShader );
    checkGlError("glAttachFragmentShader");

    // Link the GLProgram
    glLinkProgram ( programObject );
    checkGlError("glLinkProgram");

    // Check the link status
    glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

    if ( !linked )
    {
        LOGE ( "error: !linked " );
        GLint infoLen = 0;

        glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

        if ( infoLen > 1 )
        {
            char* infoLog =(char*) malloc (sizeof(char) * infoLen );

            glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
            LOGE ( "Error linking GLProgram:\n%s\n", infoLog );

            free ( infoLog );
        }

        glDeleteShader ( vertexShader );
        glDeleteShader ( fragmentShader );
        glDeleteProgram ( programObject );
        return 0;
    }

    // Free up no longer needed shader resources
    glDeleteShader ( vertexShader );
    glDeleteShader ( fragmentShader );

    return programObject;
}

GLProgram::GLProgram(const char * vertexShader, const char * fragShaderSrc) {
    mProgramId = createProgram(vertexShader, fragShaderSrc);
    mUsed = false;
}

GLuint GLProgram::getProgramId(){
    return mProgramId;
}

GLProgram::~GLProgram()
{
    if(mProgramId)
       glDeleteProgram ( mProgramId );
    mProgramId = 0;
}


/*void GLProgram::bindTexture(const char * name, texture_2d * ptexture,GLenum filter)
{
    GLint attribute;
    int usetexture;

    if(mUsed == false)
        glUseProgram(mProgramId);
    mUsed = true;
    attribute = glGetUniformLocation(mProgramId, name);
    checkGlError("glGetUniformLocation");
    usetexture = ptexture->getTexturename() - GL_TEXTURE0;
    glActiveTexture(ptexture->getTexturename());
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, ptexture->getTextureId());
    checkGlError("glBindTexture");
    glUniform1i(attribute, usetexture);
    checkGlError("glUniform1i");
}*/

void GLProgram::bindTexture(const char *name, GLuint texId, GLuint textureUnit) {
    int usetexture = textureUnit - GL_TEXTURE0;
    GLint attribute = glGetUniformLocation(mProgramId, name);
    checkGlError("glGetUniformLocation");
    glActiveTexture(textureUnit);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, texId);
    checkGlError("glBindTexture");
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
//    checkGlError("glTexParameteri");
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
//    checkGlError("glTexParameteri");
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    checkGlError("glTexParameteri");
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    checkGlError("glTexParameteri");
    glUniform1i(attribute, usetexture);
    checkGlError("glUniform1i");
}

void GLProgram::unbindTexture(GLuint textureUnit){
    glActiveTexture(textureUnit);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, 0);
    checkGlError("glBindTexture");
}

bool GLProgram::hasUniform( const char * name ) {
    return glGetUniformLocation(mProgramId, name) != -1;
}

void GLProgram::setUniformv(char* name, float value[], int length)
{
    int handler = glGetUniformLocation(mProgramId, name);
    if (handler == -1)
        return;

    switch (length) {
        case 1 :
            glUniform1fv(handler, 1, value);
            break;
        case 2 :
            glUniform2fv(handler, 1, value);
            break;
        case 3 :
            glUniform3fv(handler, 1, value);
            break;
        case 4 :
            glUniform4fv(handler, 1, value);
            break;
        case 16 :
            glUniformMatrix4fv(handler, 1, false, value);
            break;
    }
}

void GLProgram::setUniform1i( const char * name, int value ) {
    GLint location = glGetUniformLocation(mProgramId, name);
    if (location >= 0) {
        glUniform1i(location, value);
    }
}


void GLProgram::setUniform1f( const char * name, float value ) {
    GLint location = glGetUniformLocation(mProgramId, name);
    if (location >= 0) {
        glUniform1f(location, value);
    }
}


void GLProgram::setUniform2f( const char * name, float x, float y ) {
    GLint location = glGetUniformLocation(mProgramId, name);
    if (location >= 0) {
        glUniform2f(location, x, y);
    }
}


void GLProgram::setUniform3f( const char * name, float x, float y, float z ) {
    GLint location = glGetUniformLocation(mProgramId, name);
    if (location >= 0) {
        glUniform3f(location, x, y, z);
    }
}

void GLProgram::useProgram()
{

    if(mUsed == false)
        glUseProgram(mProgramId);
    mUsed = true;
}
