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

#ifndef DEBUG_HELPERS_HPP
#define DEBUG_HELPERS_HPP

#include <opencv2/opencv.hpp>

#include <string>
#include <sstream>

namespace cv {
    // This function used to show and save the image to the disk (used for during chapter writing).
    inline void showAndSave(std::string name, const cv::Mat &m) {
        // cv::imshow(name, m);
        // cv::imwrite(name + ".png", m);
        //cv::waitKey(25);
    }

    // Draw matches between two images,这个函数就是给定一个模板图，一个测试图，这两幅图所有的角点，找到的匹配集合，要画出的匹配数量。就是把两幅图上匹配的点都用绿色直线连起来。
    inline cv::Mat getMatchesImage(cv::Mat query, cv::Mat pattern,
                                   const std::vector<cv::KeyPoint> &queryKp,
                                   const std::vector<cv::KeyPoint> &trainKp,
                                   std::vector<cv::DMatch> matches, int maxMatchesDrawn) {
        cv::Mat outImg;

        if (matches.size() > maxMatchesDrawn) {
            matches.resize(maxMatchesDrawn);
        }

        cv::drawMatches//给定两幅图像，绘制寻找到的特征关键点及其匹配
                (
                        query,
                        queryKp,
                        pattern,
                        trainKp,
                        matches,
                        outImg,
                        cv::Scalar(0, 200, 0, 255),
                        cv::Scalar::all(-1),
                        std::vector<char>(),
                        cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS//单独的没匹配到的角点不绘制。
                );

        return outImg;
    }
}

namespace ml_ar {
    // Does lexical cast of the input argument to string
    template<typename T>
    std::string ToString(const T &value) {
        std::ostringstream stream;
        stream << value;
        return stream.str();
    }

}

#endif
