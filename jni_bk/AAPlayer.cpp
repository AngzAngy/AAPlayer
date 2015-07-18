/*
* This proprietary software may be used only as
* authorised by a licensing agreement from ARM Limited
* (C) COPYRIGHT 2012 ARM Limited
* ALL RIGHTS RESERVED
* The entire notice above must be reproduced on all authorised
* copies and copies may only be made to the extent permitted
* by a licensing agreement from ARM Limited.
*/

/**
* \file Triangle.cpp
* \brief A sample which shows how to draw a simple triangle to the screen.
*
* Uses a simple shader to fill the the triangle with a gradient color.
*/

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <thread>

#include "Player.h"
#include "jnilogger.h"

#include "esUtil.h"

#include "GLRGBARender.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/pixfmt.h"
#ifdef __cplusplus
}
#endif

enum {
	PLAY_STATE, PAUST_STATE, STOP_STATE
};

#define WINDOW_W 800
#define WINDOW_H 600

using namespace std;

#define Decode_Format AV_PIX_FMT_RGBA

AVFormatContext *formatctx = NULL;
AVCodecContext *codecctx = NULL;
int videoStream;
AVFrame *decodedFrame = NULL;

int displayWidth = 0;
int displayHeight = 0;
SwsContext *swsctx = NULL;
int videoState;

GLRender *gRender = NULL;

bool setDataSource(const char *fileName){
	AVCodec *pCodec = NULL;
	AVDictionary *pDict = NULL;

	AVCodec *paudioCodec = NULL;
	AVDictionary *paudioDict = NULL;

	av_register_all();

	if (avformat_open_input(&formatctx, fileName, NULL, NULL) < 0){
		return false;
	}
	if (avformat_find_stream_info(formatctx, NULL) < 0){
		return false;
	}
	av_dump_format(formatctx, 0, fileName, 0);

	videoStream = -1;
	for (int i = 0; i<formatctx->nb_streams; i++){
		if (formatctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			break;
		}
	}
	if (-1 == videoStream){
		return false;
	}

	codecctx = formatctx->streams[videoStream]->codec;
	pCodec = avcodec_find_decoder(codecctx->codec_id);
	if (NULL == pCodec) {
		return false;
	}

	if (avcodec_open2(codecctx, pCodec, &pDict) < 0){
		return false;
	}

	decodedFrame = av_frame_alloc();
	if (NULL == decodedFrame){
		return false;
	}
	return true;
}

bool setDisplaySize() {
	if (displayWidth <= 0){
		displayWidth = codecctx->width;
	}
	if (displayHeight <= 0){
		displayHeight = codecctx->height;
	}
	int dspW = displayWidth;
	int dspH = displayHeight;
	swsctx = sws_getContext(
		codecctx->width,
		codecctx->height,
		codecctx->pix_fmt,
		dspW,
		dspH,
		Decode_Format,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL);
	if (NULL == swsctx){
		return false;
	}

	return true;
}

void decodefn(){
	bool loop = true;
	AVPacket packet;
	AVPicture pict;
	Image img;
	int frameFinished;
	while (loop){
		switch (videoState){
		case PLAY_STATE:
			if (av_read_frame(formatctx, &packet) >= 0){
				if (packet.stream_index == videoStream){
					avcodec_decode_video2(codecctx, decodedFrame, &frameFinished, &packet);
					if (frameFinished){

						gRender->lockBuf(0, &img);
						pict.data[0] = (uint8_t *)img.plane[0];
						pict.linesize[0] = img.pitch[0];

						sws_scale(
							swsctx,
							(uint8_t const * const *)decodedFrame->data,
							decodedFrame->linesize,
							0,
							codecctx->height,
							pict.data,
							pict.linesize
							);
						gRender->unlockBuf(0);
					}
				}
				av_free_packet(&packet);
			}else{
				loop = false;
			}
			break;
		case PAUST_STATE:
			break;
		case STOP_STATE:
			loop = false;
			break;
		}
	}
}

void myplay() {
	videoState = PLAY_STATE;

    thread decodeThread(decodefn);
	decodeThread.detach();//This is important in windows
}

///
// Initialize the shader and program object
//
int Init(ESContext *esContext)
{
	char vertexShader[] = "attribute vec4 aPosition;"
		"attribute vec2 aTextureCoord;"
		"varying vec2 vTextureCoord;"
		"void main() {"
		" gl_Position = aPosition; "
		"vTextureCoord = aTextureCoord;}";

	char fragShaderSrc[] = "precision mediump float;"
		"varying vec2 vTextureCoord;"
		"uniform sampler2D texture;"
		"void main() {"
		"gl_FragColor = texture2D(texture, vTextureCoord);}";

	if (gRender){
		gRender->createGLProgram(vertexShader, fragShaderSrc);
		gRender->createTexture(NULL, displayWidth, displayHeight, GL_RGBA, GL_TEXTURE0);
	}

	glViewport(0, 0, displayWidth, displayHeight);
	glEnable(GL_TEXTURE);
	return TRUE;
}

void Draw(ESContext *esContext)
{
	if (gRender){
		gRender->render();
	}
	eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
}

int main(void)
{
	displayWidth = 1024;
	displayHeight = 512;
	setDataSource("F:\\ffmpeg\\hadoop.mp4");
	setDisplaySize();

	if (gRender == NULL){
		gRender = new GLRGBARender();
	}
	
	gRender->allocBuf(0, displayWidth, displayHeight);

	myplay();

	ESContext esContext;

	esInitContext(&esContext);

	esCreateWindow(&esContext, "Hello Triangle", displayWidth, displayHeight, ES_WINDOW_RGB);

	if (!Init(&esContext))
		return 0;
	
	esRegisterDrawFunc(&esContext, Draw);

	esMainLoop(&esContext);

	return 0;
}