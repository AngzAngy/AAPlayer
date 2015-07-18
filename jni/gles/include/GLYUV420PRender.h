#include <mutex>
#include "GLRender.h"

#ifndef _Included_GLYUV420PRender
#define _Included_GLYUV420PRender

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
private:
	Image mImg;
    std::mutex mImgMutex;
};

#endif
