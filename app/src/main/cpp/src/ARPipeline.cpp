////////////////////////////////////////////////////////////////////
// File includes:
#include "ARPipeline.hpp"

namespace ml_ar {

    ARPipeline::ARPipeline(const cv::Mat &patternImage, const CameraCalibration &calibration)
            : m_calibration(calibration) {
        m_patternDetector.buildPatternFromImage(patternImage, m_pattern);
        m_patternDetector.train(m_pattern);
    }

    bool ARPipeline::processFrame(const cv::Mat &inputFrame) {
        cv::Mat matchImg;
        bool patternFound = m_patternDetector.findPattern(inputFrame, m_patternInfo,
                                                          matchImg);//找到与模板图像匹配的匹配点集合，得到单应性矩阵，优化匹配组合。得到模板图像四个顶点经单应性变换得到的四个顶点二维坐标。

        if (patternFound) {
            matched = matchImg.clone();
            m_patternInfo.computePose(m_pattern, m_calibration);//得到标识在相机坐标系下的位姿矩阵，作为视景矩阵。
        }

        return patternFound;
    }

    const Transformation &ARPipeline::getPatternLocation() const {
        return m_patternInfo.pose3d;
    }
}//namespace ml_ar
