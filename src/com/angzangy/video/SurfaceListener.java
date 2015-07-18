package com.angzangy.video;

import android.graphics.SurfaceTexture;


public interface SurfaceListener {
    public void onSurfaceCreated(SurfaceTexture surfaceTexture);
    public void onSurfaceChanged(int width, int height);
}
