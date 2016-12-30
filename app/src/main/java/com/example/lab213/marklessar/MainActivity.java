package com.example.lab213.marklessar;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity implements GLSurfaceView.Renderer, CameraBridgeViewBase.CvCameraViewListener2 {
    private static String TAG="Markless_AR";

    private static final int MESS_SHOW_SOURCE=0x000b0001;
    private static final int MESS_DISMISS_MATCHED=0x000b0002;
    private static final int MESS_EXCHANGE_MATCHED=0x000b0003;
    private static final int MESS_SHOW_TAG=0x000b0004;

    private boolean systemRunning=true;

    private boolean isPatternSetted=false;

    private boolean showSource=false;
    private boolean showMatched=false;
    private boolean showAR=false;

    TextView startTxt;
    ImageView matchedImage;
    GLSurfaceView ARGlView;

    CameraBridgeViewBase sourceImage;
    private boolean mIsJavaCamera = true;

    Bitmap matchedBit;
    Mat matchedImg;
    long imageData;
    int w=0,h=0;

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                    Log.i(TAG, "OpenCV loaded successfully");
                    break;
                default:
                    super.onManagerConnected(status);
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);// 隐藏标题
        getSupportActionBar().hide();
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);// 设置全屏
       // getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);

        initViews();
        boundClickers();

        startThread();
    }

    void initViews(){
        startTxt=(TextView)findViewById(R.id.start_txt);
        matchedImage=(ImageView)findViewById(R.id.matched_image);
        ARGlView=(GLSurfaceView)findViewById(R.id.gl_sur);
        ARGlView.setRenderer(this);

        if(mIsJavaCamera)
            sourceImage=(CameraBridgeViewBase)findViewById(R.id.source_image_java_surface_view);
        else
            sourceImage=(CameraBridgeViewBase)findViewById(R.id.source_image_native_surface_view);

        sourceImage.setCvCameraViewListener(this);
    }

    void boundClickers(){
        startTxt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startTxt.setVisibility(View.GONE);
                ARHandler.sendEmptyMessage(MESS_SHOW_SOURCE);
                sourceImage.enableView();
            }
        });
        sourceImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(!isPatternSetted){
                    NDKLoader.setPatternImage(imageData,w,h);
                    isPatternSetted=true;
                    ARHandler.obtainMessage(MESS_SHOW_TAG,"Pattern Image is Setted").sendToTarget();
                }
            }
        });
        matchedImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ARHandler.sendEmptyMessage(MESS_DISMISS_MATCHED);
            }
        });
        ARGlView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //ARHandler.sendEmptyMessage(MESS_EXCHANGE_MATCHED);
            }
        });

    }


    void startThread(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                while(systemRunning){
                    if(isPatternSetted)
                        showAR=NDKLoader.currentFrame(imageData);
                }
            }
        }).start();
        new Thread(new Runnable() {
            @Override
            public void run() {
                while(systemRunning){
                    if(showMatched){
                        matchedImg=new Mat(640,240, CvType.CV_8UC3);
                        NDKLoader.getMatchedImage(matchedImg.getNativeObjAddr());
                        matchedBit=Bitmap.createBitmap(matchedImg.cols(),matchedImg.rows(), Bitmap.Config.RGB_565);
                        Utils.matToBitmap(matchedImg,matchedBit);
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                matchedImage.setImageBitmap(matchedBit);
                            }
                        });
                    }
                }
            }
        }).start();
    }

    void resetAll(){
        ARGlView.setVisibility(View.VISIBLE);
        sourceImage.setVisibility(View.VISIBLE);
        matchedImage.setVisibility(View.INVISIBLE);
        showSource=true;
        showMatched=false;
    }

    Handler ARHandler=new Handler(){
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what){
                case MESS_SHOW_SOURCE:
                    resetAll();
                    break;
                case MESS_DISMISS_MATCHED:
                    //resetAll();
                    showMatched=false;
                    matchedImage.setVisibility(View.INVISIBLE);
                    break;
                case MESS_EXCHANGE_MATCHED:
                   // resetAll();
                    showMatched=!showMatched;
                    matchedImage.setVisibility(showMatched?View.VISIBLE:View.INVISIBLE);
                    break;
                case MESS_SHOW_TAG:
                    Toast.makeText(MainActivity.this,(String)msg.obj,Toast.LENGTH_LONG).show();
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    protected void onResume() {
        super.onResume();

        ARGlView.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_11,this,mLoaderCallback);
    }

    @Override
    protected void onPause() {
        super.onPause();

        ARGlView.onPause();
        if(sourceImage!=null)
            sourceImage.disableView();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if(sourceImage!=null)
            sourceImage.disableView();

        systemRunning=false;
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {

    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int i, int i1) {

    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        if(showAR){
            NDKLoader.glesRender();
        }
    }

    @Override
    public void onCameraViewStarted(int width, int height) {

    }

    @Override
    public void onCameraViewStopped() {

    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        Mat im=inputFrame.rgba();
        synchronized (im){
            imageData=im.clone().getNativeObjAddr();
        }
        w=im.cols();
        h=im.rows();
        return im;
    }
}
