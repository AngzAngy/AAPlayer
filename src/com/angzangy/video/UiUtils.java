package com.angzangy.video;

import android.app.Activity;
import android.util.DisplayMetrics;
import android.view.Window;
import android.view.WindowManager;

public class UiUtils {
    private static int scrnWidth;
    private static int scrnHeight;
    public static void requestFullScreen(Activity activity){
        activity.requestWindowFeature(Window.FEATURE_NO_TITLE);
        activity.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
    }
    public static void init(Activity activity){
        DisplayMetrics mtr = new DisplayMetrics();
        activity.getWindowManager().getDefaultDisplay().getMetrics(mtr);
        scrnWidth=mtr.widthPixels;
        scrnHeight=mtr.heightPixels;
    }
    public static int screenWidth(){return scrnWidth;}
    public static int screenHeight(){return scrnHeight;}
}
