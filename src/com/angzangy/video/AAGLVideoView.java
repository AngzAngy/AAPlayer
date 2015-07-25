package com.angzangy.video;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.ViewGroup;

import com.angzangy.jni.GLVideoJni;

public class AAGLVideoView extends GLSurfaceView {

    MyRenderer mRenderer;

    public AAGLVideoView(Context context) {
        this(context, null);
    }

    public AAGLVideoView(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        init();
    }

    private void init() {
        GLVideoJni.init();

        setEGLContextClientVersion(2);
        mRenderer = new MyRenderer();
        setRenderer(mRenderer);
    }

    public void pause(){
        GLVideoJni.pause();
    }

    public void destroy(){
        GLVideoJni.stop();
        GLVideoJni.release();
    }

    private Runnable mRelayoutRunnable = new Runnable() {
        @Override
        public void run() {
            int videoWidth = GLVideoJni.getVideoWidth();
            int videoHeight = GLVideoJni.getVideoHeight();
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
            ViewGroup.LayoutParams params = getLayoutParams();
            params.width = GLVideoJni.getVideoWidth();
            params.height = GLVideoJni.getVideoHeight();
            setLayoutParams(params);

            GLVideoJni.start();
        }
    };
    
    public void setDataSource(String fn){
        GLVideoJni.setDataSource(fn);
        GLVideoJni.prepare();
        GLVideoJni.start();
//        post(mRelayoutRunnable);
    }

    class MyRenderer implements GLSurfaceView.Renderer{
        public MyRenderer() {
        }
        public void onSurfaceCreated(GL10 glUnused, EGLConfig config) {
            GLVideoJni.surfaceCreated();
        }
        
        public void onSurfaceChanged(GL10 glUnused, int width, int height) {
            GLVideoJni.surfaceChanged(width, height);
        }
    
        public void onDrawFrame(GL10 glUnused) {
            GLVideoJni.render();
        }
    
        private void checkGlError(String op) {
            int error;
            while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
                Log.e(AAGLVideoView.class.getSimpleName(), op + ": glError " + error);
                throw new RuntimeException(op + ": glError " + error);
            }
        }
    }
}