package com.angzangy.jni;

public class GLVideoJni {
    static {
        System.loadLibrary("aamediaplayer");
    }

    public static native void init();

    public static native void surfaceCreated();
    public static native void surfaceChanged(int width, int height);
    public static native void surfaceDestroyed();
    public static native void render();

    public static native void setDataSource(String fn);
    public static native void prepare();
    public static native int getVideoWidth();
    public static native int getVideoHeight();
    public static native void start();
    public static native void pause();
    public static native void stop();
    public static native void release();
}
