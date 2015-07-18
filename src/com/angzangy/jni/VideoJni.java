package com.angzangy.jni;

import android.view.Surface;

public class VideoJni {
    static {
        System.loadLibrary("aamediaplayer");
    }

    public static native void init();
    public static native void setSurface(Surface surface);
    public static native void surfaceDestroyed();
    public static native void setDataSource(String fn);
    public static native void prepare();
    public static native int getVideoWidth();
    public static native int getVideoHeight();
    public static native void start();
    public static native void pause();
    public static native void stop();
    public static native void release();
}
