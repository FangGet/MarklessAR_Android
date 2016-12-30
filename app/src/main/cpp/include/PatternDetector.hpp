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

#ifndef EXAMPLE_MARKERLESS_AR_PATTERNDETECTOR_HPP
#define EXAMPLE_MARKERLESS_AR_PATTERNDETECTOR_HPP

////////////////////////////////////////////////////////////////////
// File includes:
#include "Pattern.hpp"

#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp>

namespace ml_ar {

    class PatternDetector {
    public:
        /**
        * Initialize a pattern detector with specified feature detector, descriptor extraction and matching algorithm
        */
        PatternDetector
                (
                        cv::Ptr<cv::FeatureDetector> detector = new cv::ORB(
                                1000), //FeatureDetetor是虚类，通过定义FeatureDetector的对象可以使用多种特征检测方法,ORB是一种检测算法。ORB(1000)是构造函数，初始化参数。感觉这句话就是新建一个基于ORB的特征检测类的实例：detector
                        cv::Ptr<cv::DescriptorExtractor> extractor = new cv::FREAK(false,
                                                                                   false), //DescriptorExtractor 的子类都是描述子提取器，包括FRAKE。// 用Freak特征来描述特征点
                        cv::Ptr<cv::DescriptorMatcher> matcher = new cv::BFMatcher(cv::NORM_HAMMING,
                                                                                   true),// 特征匹配，计算Hamming距离,这个已经支持交叉验证，建立 cv::BFMatcher将第二参数声明为true
                        bool enableRatioTest = false
                );

        /**
        *
        */
        void train(const Pattern &pattern);//使用模板图像训练描述符匹配器

        /**
        * Initialize Pattern structure from the input image.
        * This function finds the feature points and extract descriptors for them.
        *初始化图案结构从输入图像中，这个结构找到特征点，然后提取它们
        */
        void buildPatternFromImage(const cv::Mat &image, Pattern &pattern) const;

        /**
        * Tries to find a @pattern object on given @image.
        * The function returns true if succeeded and store the result (pattern 2d location, homography) in @info.
        *在一个给定的图像上试图找到模板的目标，如果成功，这个函数返回true，保存模板的2D位置，和单应性矩阵在info中。
        */
        bool findPattern(const cv::Mat &image, PatternTrackingInfo &info,
                         cv::Mat &matched);//细化单应性矩阵，有利于找到更精确的单应变换。

        bool enableRatioTest;
        bool enableHomographyRefinement;
        //精确单应性矩阵使能？
        float homographyReprojectionThreshold;//应该是单应性矩阵重透射阈值

    protected:

        bool extractFeatures(const cv::Mat &image, std::vector<cv::KeyPoint> &keypoints,
                             cv::Mat &descriptors) const;//keypoint只是保存了检测到的特征点的一些基本信息，提取出来的特征向量其实不是在这个里面，特征向量通过DescriptorExtractor 提取，结果放在一个Mat的数据结构中。这个数据结构才真正保存了该特征点所对应的特征向量。

        void getMatches(const cv::Mat &queryDescriptors,
                        std::vector<cv::DMatch> &matches);//第一个参数是查询描述子矩阵，DMatch用于匹配特征关键点的特征描述子的类：查询特征描述子索引, 特征描述子索引, 训练图像索引, 以及不同特征描述子之间的距离.交叉匹配过滤，判定匹配的准确度。

        /**
        * Get the gray image from the input image.
        * Function performs necessary color conversion if necessary
        * Supported input images types - 1 channel (no conversion is done), 3 channels (assuming BGR) and 4 channels (assuming BGRA).
        */
        static void getGray(const cv::Mat &image, cv::Mat &gray);//灰度化

        /**
        *
        */
        static bool refineMatchesWithHomography(//用RANSIC算法进行单应性矩阵匹配，目的是过滤在几何上不正确的匹配。细化匹配。
                const std::vector<cv::KeyPoint> &queryKeypoints,
                const std::vector<cv::KeyPoint> &trainKeypoints,
                float reprojectionThreshold,
                std::vector<cv::DMatch> &matches,
                cv::Mat &homography);

    private:
        std::vector<cv::KeyPoint> m_queryKeypoints;
        cv::Mat m_queryDescriptors;
        std::vector<cv::DMatch> m_matches;
        std::vector<std::vector<cv::DMatch> > m_knnMatches;

        cv::Mat m_grayImg;
        cv::Mat m_warpedImg;
        cv::Mat m_roughHomography;
        cv::Mat m_refinedHomography;

        Pattern m_pattern;
        cv::Ptr<cv::FeatureDetector> m_detector;
        cv::Ptr<cv::DescriptorExtractor> m_extractor;
        cv::Ptr<cv::DescriptorMatcher> m_matcher;
    };

}

#endif
