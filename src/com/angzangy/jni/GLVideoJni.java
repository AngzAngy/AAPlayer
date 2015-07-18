package com.angzangy.jni;


public class GLVideoJni {
    static {
        System.loadLibrary("aamediaplayer");
    }
    private int mVideoWidth;
    private int mVideoHeight;

    public GLVideoJni(){
        nativeInit(this);
    }

    public void setDataSource(String fileName){
        if(fileName!=null){
            nativeSetDataSource(fileName);
        }
    }

    public void setDisplaySize(int displayWidth, int displayHeight){
        nativeSetDiplaySize(displayWidth, displayHeight);
    }

    public int getVideoWidth(){
        return mVideoWidth;
    }

    public int getVideoHeight(){
        return mVideoHeight;
    }

    public void play(){
        nativePlay();
    }
    public void pause(){
        nativePause();
    }
    public void stop(){
        nativeStop();
    }

    public void release(){
        nativeRelase();
    }

    public void onSurfaceCreated(){
        nativeOnSurfaceCreated();
    }

    public void onSurfaceChanged(int width, int height){
        nativeOnSurfaceChanged(width, height);
    }

    public void onRender(){
        nativeOnRender();
    }

    private static native void nativeOnSurfaceCreated();
    private static native void nativeOnSurfaceChanged(int width, int height);
    private static native void nativeOnRender();

    private static native void nativeInit(GLVideoJni thiz);
    private static native void nativeSetDataSource(String fileName);
    private static native void nativeSetDiplaySize(int width, int height);
    private static native void nativePlay();
    private static native void nativePause();
    private static native void nativeStop();
    private static native void nativeRelase();
}
