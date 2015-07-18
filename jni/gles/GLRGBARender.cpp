#include "GLRGBARender.h"

GLRGBARender::GLRGBARender(){
	memset(mRgbaImg, 0, sizeof(Image)*RGBA_IMG_NUM);
}

GLRGBARender::~GLRGBARender(){
    for(int i=0;i<RGBA_IMG_NUM;i++){
        freeBuf(i);
    }
}

void GLRGBARender::perRender(){
	Image img;
    lockBuf(0, &img);
    if(mTextures[0] && img.plane[0]!=NULL){
		mTextures[0]->subImage((const GLvoid*)img.plane[0], 0, 0, img.width, img.height, GL_RGBA);
        mProgram->bindTexture("texture", mTextures[0]->getTextureId(), mTextures[0]->getTextureUnit());
    }
    unlockBuf(0);
}

void GLRGBARender::postRender(){
    if(mTextures[0]){
        mProgram->unbindTexture(mTextures[0]->getTextureUnit());
    }
}

void GLRGBARender::allocBuf(const int bufIdx,
        const uint32_t bufWidth, const uint32_t bufHeight){
    if(bufIdx<RGBA_IMG_NUM){
        mImgMutex[bufIdx].lock();

        if(mRgbaImg[bufIdx].plane[0]){
			free(mRgbaImg[bufIdx].plane[0]);
        }
		mRgbaImg[bufIdx].pitch[0] = bufWidth * 4;
        mRgbaImg[bufIdx].width = bufWidth;
        mRgbaImg[bufIdx].height = bufHeight;
		mRgbaImg[bufIdx].plane[0] = malloc(mRgbaImg[bufIdx].pitch[0] * mRgbaImg[bufIdx].height);

        mImgMutex[bufIdx].unlock();
    }
}

void GLRGBARender::freeBuf(const int bufIdx){
    if(bufIdx<RGBA_IMG_NUM){
        mImgMutex[bufIdx].lock();

		if (mRgbaImg[bufIdx].plane[0]){
			free(mRgbaImg[bufIdx].plane[0]);
        }
		memset(&(mRgbaImg[bufIdx]), 0, sizeof(Image));

        mImgMutex[bufIdx].unlock();
    }
}

bool GLRGBARender::lockBuf(const int bufIdx, Image *pImg){
	if (NULL==pImg || bufIdx>=RGBA_IMG_NUM){
		return false;
	}
    mImgMutex[bufIdx].lock();
	//*pImg = mRgbaImg[bufIdx];
	memcpy(pImg, &(mRgbaImg[bufIdx]), sizeof(Image));
    return true;
}

bool GLRGBARender::unlockBuf(const int bufIdx){
    if(bufIdx<RGBA_IMG_NUM){
        mImgMutex[bufIdx].unlock();
        return true;
    }else{
        return false;
    }
}
