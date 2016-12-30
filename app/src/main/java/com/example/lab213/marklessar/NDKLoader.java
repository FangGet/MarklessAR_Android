package com.example.lab213.marklessar;

/**
 * Created by lab213 on 16-12-26.
 */

public class NDKLoader {
    static{
        System.loadLibrary("native-lib");
    }

    //public static native int[] getGrayImage(int[] pixels, int w, int h);

    //set pattern image, start markless ar
    public static native void setPatternImage(long imgData,int w,int h);

    //start markless ar
    public static native boolean currentFrame(long imgData);

    //gles render
    public static native void glesRender();

    //get matched image
    public static native void getMatchedImage(long matchedData);

}
