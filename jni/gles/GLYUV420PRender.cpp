#include "GLYUV420PRender.h"

GLYUV420PRender::GLYUV420PRender(){
	memset(&mImg, 0, sizeof(Image));
}

GLYUV420PRender::~GLYUV420PRender(){
    freeBuf(0);
}

void GLYUV420PRender::perRender(){
//	Image img;
//    lockBuf(0, &img);
    if(mTextures[0] && mImg.plane[0]!=NULL){
        mProgram->bindTexture("ytexture", mTextures[0]->getTextureId(), mTextures[0]->getTextureUnit());
		mTextures[0]->subImage((const GLvoid*)mImg.plane[0], 0, 0, mImg.width, mImg.height, GL_LUMINANCE);
    }
    if(mTextures[1] && mImg.plane[1]!=NULL){
        mProgram->bindTexture("utexture", mTextures[1]->getTextureId(), mTextures[1]->getTextureUnit());
        mTextures[1]->subImage((const GLvoid*)mImg.plane[1], 0, 0, mImg.width/2, mImg.height/2, GL_LUMINANCE);
    }
    if(mTextures[2] && mImg.plane[2]!=NULL){
        mProgram->bindTexture("vtexture", mTextures[2]->getTextureId(), mTextures[2]->getTextureUnit());
        mTextures[2]->subImage((const GLvoid*)mImg.plane[2], 0, 0, mImg.width/2, mImg.height/2, GL_LUMINANCE);
    }
//    unlockBuf(0);
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

        int bufLen = bufWidth * bufHeight;
        uint8_t *bufaddr = (uint8_t *)malloc(bufLen * 3 / 2);

        mImg.width = bufWidth;
        mImg.height = bufHeight;

        /*
         * y buf wxh
         */
        mImg.pitch[0] = bufWidth;
        mImg.plane[0] = bufaddr;

        /*
         * u buf w/2 x h/2
         */
        mImg.pitch[1] = bufWidth / 2;
        mImg.plane[1] = bufaddr + bufLen;

        /*
         * v buf w/2 x h/2
         */
        mImg.pitch[2] = bufWidth / 2;
        mImg.plane[2] = bufaddr + (bufLen * 5 / 4);

        mImgMutex.unlock();
}

void GLYUV420PRender::freeBuf(const int bufIdx){
        mImgMutex.lock();

        if(mImg.plane[0]){
            free(mImg.plane[0]);
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

Image *GLYUV420PRender::getImagePtr(){
    return &mImg ;
}
