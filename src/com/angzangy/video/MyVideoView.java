package com.angzangy.video;

import com.angzangy.jni.VideoJni;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewGroup;

public class MyVideoView extends SurfaceView implements SurfaceHolder.Callback{
    private SurfaceHolder mHolder;
    public MyVideoView(Context context) {
        super(context);
        init();
    }

    public MyVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public MyVideoView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init();
    }

    private void init(){
        mHolder=getHolder();
        mHolder.addCallback(this);
        mHolder.setFormat(PixelFormat.RGBA_8888);
        VideoJni.init();
    }

    private Runnable mRelayoutRunnable = new Runnable() {
        @Override
        public void run() {
            int videoWidth = VideoJni.getVideoWidth();
            int videoHeight = VideoJni.getVideoHeight();
            int width,height;
            float widthScaledRatio = UiUtils.screenWidth() * 1.0f / videoWidth;
            float heightScaledRatio = UiUtils.screenHeight() * 1.0f / videoHeight;
            if (widthScaledRatio > heightScaledRatio) {
                // use heightScaledRatio
                width = (int) (videoWidth * heightScaledRatio);
                height = UiUtils.screenHeight();
            } else {
                // use widthScaledRatio
                width = UiUtils.screenWidth();
                height = (int) (videoHeight * widthScaledRatio);
            }
//            ViewGroup.LayoutParams params = getLayoutParams();
//            params.width = UiUtils.screenWidth();
//            params.height = UiUtils.screenHeight();
//            setLayoutParams(params);

            VideoJni.start();
        }
    };
    
    public void setDataSource(String fn){
        VideoJni.setDataSource(fn);
        VideoJni.setSurface(mHolder.getSurface());
        VideoJni.prepare();
        post(mRelayoutRunnable);
    }

    public int getVideoWidth(){
        return VideoJni.getVideoWidth();
    }

    public  int getVideoHeight(){
        return VideoJni.getVideoHeight();
    }
    
    public  void pause(){
        VideoJni.pause();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int arg1, int arg2, int arg3) {
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        VideoJni.stop();
        VideoJni.surfaceDestroyed();
        VideoJni.release();
    }
}
