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

////////////////////////////////////////////////////////////////////
// File includes:
#include "Pattern.hpp"

namespace ml_ar {

    void PatternTrackingInfo::computePose(const Pattern &pattern,
                                          const CameraCalibration &calibration) {
        cv::Mat Rvec;
        cv::Mat_<float> Tvec;
        cv::Mat raux, taux;

        cv::solvePnP(pattern.points3d, points2d, calibration.getIntrinsic(),
                     calibration.getDistorsion(), raux, taux);
        raux.convertTo(Rvec, CV_32F);//转换成32位浮点数
        taux.convertTo(Tvec, CV_32F);

        cv::Mat_<float> rotMat(3, 3);
        cv::Rodrigues(Rvec, rotMat);//由向量转换成3x3矩阵。

        // Copy to transformation matrix   把旋转矩阵和平移矩阵组合成位姿矩阵，在OpenGl中做视景矩阵。
        for (int col = 0; col < 3; col++) {
            for (int row = 0; row < 3; row++) {
                pose3d.r().mat[row][col] = rotMat(row, col); // Copy rotation component
            }
            pose3d.t().data[col] = Tvec(col); // Copy translation component
        }

        // Since solvePnP finds camera location, w.r.t to marker pose, to get marker pose w.r.t to the camera we invert it.
        //计算出的位姿矩阵是相机在标识坐标系下的转换矩阵，要进行转换，变成标识在相机坐标系下的位姿矩阵。
        pose3d = pose3d.getInverted();
    }

    void PatternTrackingInfo::draw2dContour(cv::Mat &image, cv::Scalar color) const {
        for (size_t i = 0; i < points2d.size(); i++) {
            cv::line(image, points2d[i], points2d[(i + 1) % points2d.size()], color, 2, CV_AA);
        }
    }

}



