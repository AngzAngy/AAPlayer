#include <mutex>
#include "GLRender.h"

#ifndef _Included_GLRGBARender
#define _Included_GLRGBARender

#define RGBA_IMG_NUM 1

class GLRGBARender: public GLRender{

public:

    GLRGBARender();
    virtual ~GLRGBARender();

    virtual void perRender();
    virtual void postRender();

    virtual void allocBuf(const int bufIdx, const uint32_t bufWidth, const uint32_t bufHeight);
    virtual void freeBuf(const int bufIdx);
	virtual bool lockBuf(const int bufIdx, Image *pImg);
    virtual bool unlockBuf(const int bufIdx);
private:
	Image mRgbaImg[RGBA_IMG_NUM];
    std::mutex mImgMutex[RGBA_IMG_NUM];
};

#endif
