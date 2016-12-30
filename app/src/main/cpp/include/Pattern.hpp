/*****************************************************************************
*   Markerless AR desktop application.
******************************************************************************
*   by Khvedchenia Ievgen, 5th Dec 2012
*   http://computer-vision-talks.com
******************************************************************************
*   Ch3 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/

#ifndef EXAMPLE_MARKERLESS_AR_PATTERN_HPP
#define EXAMPLE_MARKERLESS_AR_PATTERN_HPP

////////////////////////////////////////////////////////////////////
// File includes:
#include "GeometryTypes.hpp"
#include "CameraCalibration.hpp"

#include <opencv2/opencv.hpp>

namespace ml_ar {
    /**
  * Store the image data and computed descriptors of target pattern
  */
    struct Pattern {
        cv::Size size;
        //图像大小
        cv::Mat frame;
        //图像数据
        cv::Mat grayImg;//灰度图像

        std::vector<cv::KeyPoint> keypoints;
        //图像中特征点集
        cv::Mat descriptors;//

        std::vector<cv::Point2f> points2d;
        std::vector<cv::Point3f> points3d;
    };

    /**
    * Intermediate pattern tracking info structure
    */
    struct PatternTrackingInfo {
        cv::Mat homography;
        //单应性矩阵
        std::vector<cv::Point2f> points2d;
        //二维点集
        Transformation pose3d;//包含旋转矩阵和平移矩阵

        void draw2dContour(cv::Mat &image, cv::Scalar color) const;

        /**
        * Compute pattern pose using PnP algorithm
        */
        void computePose(const Pattern &pattern, const CameraCalibration &calibration);
    };

}//end of namespace ml_ar



#endif
