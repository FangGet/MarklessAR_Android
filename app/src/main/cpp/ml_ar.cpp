#include <jni.h>
#include <string>
#include<opencv2/opencv.hpp>

#include "ARDrawingContext.hpp"
#include "ARPipeline.hpp"
#include "DebugHelpers.hpp"

#include <android/log.h>

#define LOG_TAG "ORB_SLAM_SYSTEM"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOG(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG, __VA_ARGS__)

using namespace cv;
extern "C" {
cv::Mat *patternFrame;
JavaVM *jvm;
ml_ar::ARPipeline *pipeline;
ml_ar::ARDrawingContext *drawingCtx;
ml_ar::CameraCalibration *calibration;

cv::Size frameSize(320, 240);

bool processFrame(const cv::Mat &patternImage, ml_ar::CameraCalibration &calibration,
                  const cv::Mat &image);

JNIEXPORT
void JNICALL Java_com_example_lab213_marklessar_NDKLoader_setPatternImage(JNIEnv *env,
                                                                          jclass type,
                                                                          jlong imgData,
                                                                          jint w,
                                                                          jint h) {
    env->GetJavaVM(&jvm);
    jvm->AttachCurrentThread(&env, NULL);

    patternFrame = (cv::Mat *) imgData;
    cv::resize(*patternFrame, *patternFrame, frameSize);
    calibration = new ml_ar::CameraCalibration(454.9f, 457.9f, 127.6f, 112.2f);
    pipeline = new ml_ar::ARPipeline(*patternFrame, *calibration);
    drawingCtx = new ml_ar::ARDrawingContext(frameSize, *calibration);
}

JNIEXPORT
jboolean JNICALL Java_com_example_lab213_marklessar_NDKLoader_currentFrame(JNIEnv *env,
                                                                           jclass type,
                                                                           jlong imgData) {
    cv::Mat *im = (cv::Mat *) imgData;
    if (!im->data)
        return false;
    cv::resize(*im, *im, frameSize);
    return processFrame(*patternFrame, *calibration, *im);
}

JNIEXPORT
void JNICALL Java_com_example_lab213_marklessar_NDKLoader_glesRender(JNIEnv *env,
                                                                     jclass type) {
    drawingCtx->updateWindow();
}

JNIEXPORT
void JNICALL Java_com_example_lab213_marklessar_NDKLoader_getMatchedImage(JNIEnv *env,
                                                                          jclass type,
                                                                          jlong matchedData) {

    cv::Mat *matchedImg = (cv::Mat *) matchedData;
    cv::Mat ima = pipeline->matched;

    memcpy(matchedImg->data, ima.data, ima.cols * ima.rows * 3);

//    LOGE("ima channels:%d",ima.channels());
//    jintArray resultArray = env->NewIntArray(ima.rows * ima.cols*3);
//    jint *resultPtr;
//    resultPtr = env->GetIntArrayElements(resultArray, NULL);
//    for (int i = 0; i < ima.rows; i++)
//        for (int j = 0; j < ima.cols; j++) {
////            int R = ima.at < Vec3b> (i, j)[0];
////            int G = ima.at < Vec3b > (i, j)[1];
////            int B = ima.at < Vec3b > (i, j)[2];
//            matchedImg->at<Vec3b>(i,j)=ima.at < Vec3b> (i, j);
//        }
//    //env->SetIntArrayRegion(resultArray,0,ima.rows * ima.cols,(jint*)ima.data);
//    //env->ReleaseIntArrayElements(buf,cbuf,0);
//    env->ReleaseIntArrayElements(resultArray, resultPtr, 0);
//    return resultArray;
}

bool processFrame(const cv::Mat &patternImage, ml_ar::CameraCalibration &calibration,
                  const cv::Mat &image) {
    // Clone image used for background (we will draw overlay on it)
    cv::Mat img = image.clone();

    // Draw information:在图像上绘制一些信息。
//    if (pipeline->m_patternDetector.enableHomographyRefinement)//第一次是true,
//        cv::putText(img, "Pose refinement: On   ('h' to switch off)", cv::Point(10,15), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0,200,0));//就是在图像的某个位置添加颜色大小的字体。
//    else
//        cv::putText(img, "Pose refinement: Off  ('h' to switch on)",  cv::Point(10,15), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0,200,0));
//
//    cv::putText(img, "RANSAC threshold: " + ml_ar::ToString(pipeline->m_patternDetector.homographyReprojectionThreshold) + "( Use'-'/'+' to adjust)", cv::Point(10, 30), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0,200,0));

    // Set a new camera frame:
    drawingCtx->updateBackground(img);//把测试图像给OpenGL

    // Find a pattern and update it's detection status:
    drawingCtx->isPatternPresent = pipeline->processFrame(image);//计算位姿矩阵

    // Update a pattern pose:
    drawingCtx->patternPose = pipeline->getPatternLocation();//更新位姿矩阵，因为是一直循环的，或者在视频中，需要对每一帧都计算。

    // Request redraw of the window:
    //drawingCtx->updateWindow();//重新绘制图像。
    return true;
}


}