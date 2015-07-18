#include "GLYUV420PRender.h"

GLYUV420PRender::GLYUV420PRender(){
	memset(mImg, 0, sizeof(Image));
}

GLYUV420PRender::~GLYUV420PRender(){
    freeBuf(0);
}

void GLYUV420PRender::perRender(){
	Image img;
    lockBuf(0, &img);
    if(mTextures[0] && img.plane[0]!=NULL){
		mTextures[0]->subImage((const GLvoid*)img.plane[0], 0, 0, img.width, img.height, GL_LUMINANCE);
        mProgram->bindTexture("ytexture", mTextures[0]->getTextureId(), mTextures[0]->getTextureUnit());
    }
    if(mTextures[1] && img.plane[1]!=NULL){
        mTextures[1]->subImage((const GLvoid*)img.plane[1], 0, 0, img.width, img.height/4, GL_LUMINANCE);
        mProgram->bindTexture("utexture", mTextures[1]->getTextureId(), mTextures[1]->getTextureUnit());
    }
    if(mTextures[2] && img.plane[2]!=NULL){
        mTextures[2]->subImage((const GLvoid*)img.plane[2], 0, 0, img.width, img.height/4, GL_LUMINANCE);
        mProgram->bindTexture("vtexture", mTextures[2]->getTextureId(), mTextures[2]->getTextureUnit());
    }
    unlockBuf(0);
}

void GLYUV420PRender::postRender(){
    if(mTextures[0]){
        mProgram->unbindTexture(mTextures[0]->getTextureUnit());
    }
    if(mTextures[1]){
        mProgram->unbindTexture(mTextures[1]->getTextureUnit());
    }
    if(mTextures[2]){
        mProgram->unbindTexture(mTextures[2]->getTextureUnit());
    }
}

void GLYUV420PRender::allocBuf(const int bufIdx,
        const uint32_t bufWidth, const uint32_t bufHeight){
        mImgMutex.lock();

        if(mImg.plane[0]){
			free(mImg.plane[0]);
        }
        if(mImg.plane[1]){
            free(mImg.plane[1]);
        }
        if(mImg.plane[2]){
            free(mImg.plane[2]);
        }
        mImg.width = bufWidth;
        mImg.height = bufHeight;

        mImg.pitch[0] = bufWidth;
        mImg.plane[0] = malloc(mImg.pitch[0] * mImg.height);

        mImg.pitch[1] = bufWidth;
        mImg.plane[1] = malloc(mImg.pitch[1] * mImg.height / 4);

        mImg.pitch[2] = bufWidth;
        mImg.plane[2] = malloc(mImg.pitch[2] * mImg.height / 4);

        mImgMutex.unlock();
}

void GLYUV420PRender::freeBuf(const int bufIdx){
        mImgMutex.lock();

        if(mImg.plane[0]){
            free(mImg.plane[0]);
        }
        if(mImg.plane[1]){
            free(mImg.plane[1]);
        }
        if(mImg.plane[2]){
            free(mImg.plane[2]);
        }

		memset(&(mImg), 0, sizeof(Image));

        mImgMutex.unlock();
}

bool GLYUV420PRender::lockBuf(const int bufIdx, Image *pImg){

    mImgMutex.lock();
	memcpy(pImg, &(mImg), sizeof(Image));

    return true;
}

bool GLYUV420PRender::unlockBuf(const int bufIdx){
     mImgMutex.unlock();
     return true;
}
