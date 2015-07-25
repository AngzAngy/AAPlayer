package com.angzangy.video;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class VideoActivity extends Activity implements View.OnClickListener
{
    public static final String PLAY_VIDEO_INFO = "pl_v_info";
//    private GLSurfaceView videoGlView;
//    private MyVideoView mVideoView;
    private AAGLVideoView mVideoView;
    private Button button;
//    private GLVideoJni videoEngine;
    private boolean is;
    private String mVideoFilePath;
    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        UiUtils.requestFullScreen(this);
        UiUtils.init(this);
        setContentView(R.layout.video_player_layout);
        Intent intent=getIntent();
        String videoFilePath = null;
        if(intent!=null){
            videoFilePath=intent.getStringExtra(PLAY_VIDEO_INFO);
        }
//        videoGlView = (GLSurfaceView) findViewById(R.id.video_glview);
//        videoGlView.setEGLContextClientVersion(2);
//        videoGlView.setRenderer(new MyRenderer());
        button = (Button) findViewById(R.id.buttonStart);
        button.setOnClickListener(this);
        findViewById(R.id.buttonPaust).setOnClickListener(this);
        mVideoView=(AAGLVideoView)findViewById(R.id.video_view);
        mVideoFilePath = videoFilePath;

//        videoGlView = (GLSurfaceView) findViewById(R.id.video_glview);
//        videoGlView.setEGLContextClientVersion(2);
//        videoGlView.setRenderer(new MyRenderer());
//        if(videoFilePath!=null){
//            videoEngine=new GLVideoJni();
//            videoEngine.setDataSource(videoFilePath);
//
//            int videoWidth = videoEngine.getVideoWidth();
//            int videoHeight = videoEngine.getVideoHeight();
//            int width,height;
//            float widthScaledRatio = UiUtils.screenWidth() * 1.0f / videoWidth;
//            float heightScaledRatio = UiUtils.screenHeight() * 1.0f / videoHeight;
//            if (widthScaledRatio > heightScaledRatio) {
//                // use heightScaledRatio
//                width = (int) (videoWidth * heightScaledRatio);
//                height = UiUtils.screenHeight();
//            } else {
//                // use widthScaledRatio
//                width = UiUtils.screenWidth();
//                height = (int) (videoHeight * widthScaledRatio);
//            }
//            videoEngine.setDisplaySize(width, height);
//            updateSurfaceView(videoGlView, width, height);
//        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.buttonStart:
//                if(!is && videoEngine!=null){
//                    is = true;
//                    videoEngine.play();
//                }
                if(mVideoFilePath!=null){
                    mVideoView.setDataSource(mVideoFilePath);
                }
                break;
            case R.id.buttonPaust:
                mVideoView.pause();
                break;
        }
    }

    public void onDestroy(){
        mVideoView.destroy();
    }
//    private void updateSurfaceView(View view, int pWidth, int pHeight) {
//        ViewGroup.LayoutParams params = view.getLayoutParams();
//        params.width = pWidth;
//        params.height = pHeight;
//        view.setLayoutParams(params);
//    }


//    class MyRenderer implements GLSurfaceView.Renderer{
//
//        public MyRenderer() {
//        }
//
//        public void onDrawFrame(GL10 glUnused) {
////            videoEngine.onRender();
//        }
//
//        public void onSurfaceChanged(GL10 glUnused, int width, int height) {
//            GLES20.glViewport(0, 0, width, height);
////            videoEngine.onSurfaceChanged(width, height);
//        }
//
//        public void onSurfaceCreated(GL10 glUnused, EGLConfig config) {
//            GLES20.glEnable(GLES20.GL_TEXTURE);
//            GLES20.glEnable(GLES20.GL_BLEND);
//            GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);
//            GLES20.glClearColor(0.643f, 0.776f, 0.223f, 1.0f);
////            videoEngine.onSurfaceCreated();
//        }
//    }
}
