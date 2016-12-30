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
#include "PatternDetector.hpp"
#include "DebugHelpers.hpp"

////////////////////////////////////////////////////////////////////
// Standard includes:
#include <cmath>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <cassert>

namespace ml_ar {

    PatternDetector::PatternDetector(cv::Ptr<cv::FeatureDetector> detector,
                                     cv::Ptr<cv::DescriptorExtractor> extractor,
                                     cv::Ptr<cv::DescriptorMatcher> matcher,
                                     bool ratioTest)
            : m_detector(detector), m_extractor(extractor), m_matcher(matcher),
              enableRatioTest(ratioTest), enableHomographyRefinement(true),
              homographyReprojectionThreshold(3) {
    }

    void PatternDetector::buildPatternFromImage(const cv::Mat &image, Pattern &pattern) const {
        int numImages = 4;
        float step = sqrtf(2.0f);//步长是2的平方根

        // Store original image in pattern structure
        pattern.size = cv::Size(image.cols, image.rows);
        pattern.frame = image.clone();//复制整个Mat的数据，不只是Mat的矩阵头和矩阵指针
        getGray(image, pattern.grayImg);//灰度化输入图像，这是整个程序的开始过程啊，感觉！！！！

        // Build 2d and 3d contours (3d contour lie in XY plane since it's planar)
        pattern.points2d.resize(
                4);//2D边缘向量的大小是4，这里感觉是把point2d的容器大小变为4,使用默认的构造函数构造两个新的元素。这里的容器里的元素是一个个的二维点
        pattern.points3d.resize(4);//3D边缘是XY平面，因为是平面的。这个是三维点的容器。

        // Image dimensions图像尺寸
        const float w = image.cols;
        const float h = image.rows;

        // Normalized dimensions:标准化尺寸
        const float maxSize = std::max(w, h);
        const float unitW = w / maxSize;
        const float unitH = h / maxSize;

        //这里和mark一样处理成大小已知的图案。得到模板图案的大小，四个顶角点的相对坐标。在下面将计算经单应性矩阵变换的四个顶点坐标，就是在测试图像上的四个顶点坐标
        pattern.points2d[0] = cv::Point2f(0, 0);
        pattern.points2d[1] = cv::Point2f(w, 0);
        pattern.points2d[2] = cv::Point2f(w, h);
        pattern.points2d[3] = cv::Point2f(0, h);
        //同理是模板图案坐标系统的四个角点的三维相对坐标。直接用于计算旋转矩阵和平移矩阵。
        pattern.points3d[0] = cv::Point3f(-unitW, -unitH, 0);
        pattern.points3d[1] = cv::Point3f(unitW, -unitH, 0);
        pattern.points3d[2] = cv::Point3f(unitW, unitH, 0);
        pattern.points3d[3] = cv::Point3f(-unitW, unitH, 0);

        extractFeatures(pattern.grayImg, pattern.keypoints, pattern.descriptors);//图像特征提取，看下面的程序分析
    }

    //使用指定的角点检测算法对输入的模板图像检测出角点，存在容器里。由检测出的角点使用特征提取算法计算特征描述子，都保存在pattern结构体中。
    bool PatternDetector::extractFeatures(const cv::Mat &image,
                                          std::vector<cv::KeyPoint> &keypoints,
                                          cv::Mat &descriptors) const {
        assert(!image.empty());
        assert(image.channels() == 1);

        m_detector->detect(image, keypoints);//检测模板图案中的角点，保存在keypoints中，这是一个角点的容器。
        if (keypoints.empty())
            return false;

        m_extractor->compute(image, keypoints, descriptors);//根据检测到的图像中的关键点计算描述子.
        if (keypoints.empty())//这里是不是应该是descriptors
            return false;

        return true;
    }

    //就是把特征描述子加入到匹配器中，然后对这些描述子进行训练，训练一个描述子匹配器，默认是flann，训练这个算法的内部结构，应该更适合这个描述子。
    void PatternDetector::train(const Pattern &pattern) {
        // Store the pattern object
        //保存模板图像数据结构体，有角点数据，描述子数据。
        m_pattern = pattern;

        // API of cv::DescriptorMatcher is somewhat tricky
        // First we clear old train data:
        //首先清除训练数据
        m_matcher->clear();//清空特征描述子训练集.

        // Then we add vector of descriptors (each descriptors matrix describe one image).
        // This allows us to perform search across multiple images:
        //然后增加描述符向量，包含很多描述符，每一个描述符矩阵描述一个图像，这可以让我们在多个图像中进行搜索，当下就用了一个描述符
        std::vector<cv::Mat> descriptors(1);
        descriptors[0] = pattern.descriptors.clone(); //把模板图像的描述符复制给描述符矩阵
        m_matcher->add(descriptors);//在描述符匹配器中增加描述符。增加特征描述子用于特征描述子集训练.

        // After adding train data perform actual train:
        m_matcher->train();//利用指定的描述符匹配器进行训练模板匹配器，训练一个特征描述子匹配器，在匹配之前都要先进行训练的。
    }


    bool PatternDetector::findPattern(const cv::Mat &image, PatternTrackingInfo &info,
                                      cv::Mat &matched) {
        // Convert input image to gray
        getGray(image, m_grayImg);

        // Extract feature points from input gray image，这里是计算测试图像的角点和特征描述子。
        extractFeatures(m_grayImg, m_queryKeypoints, m_queryDescriptors);

        // Get matches with current pattern
        getMatches(m_queryDescriptors, m_matches);//对测试图像的特征描述子，在训练描述子中找到一个最佳匹配，二者组成一个匹配，放在匹配集中。

        cv::Mat tmp = image.clone();

        // Find homography transformation and detect good matches，若找到八组以上优化后的匹配，就返回1.
        bool homographyFound = refineMatchesWithHomography(//计算单应性矩阵，然后根据是否是离群值得到最后的优化匹配组。最后一个参数是单应性矩阵
                m_queryKeypoints,
                m_pattern.keypoints,
                homographyReprojectionThreshold,
                m_matches,
                m_roughHomography);

        if (homographyFound) {
            // If homography refinement enabled improve found transformation
            if (enableHomographyRefinement)//是否进行二次单应性变换
            {
                // Warp image using found homography 使用单应性矩阵进行透视变换
                //第四个参数是若指定 matrix 是输出图像到输入图像的反变换，因此可以直接用来做象素插值。否则, 函数从 map_matrix 得到反变换。双三次插值。
                cv::warpPerspective(m_grayImg, m_warpedImg, m_roughHomography, m_pattern.size,
                                    cv::WARP_INVERSE_MAP | cv::INTER_CUBIC);
                // Get refined matches:
                std::vector<cv::KeyPoint> warpedKeypoints;
                std::vector<cv::DMatch> refinedMatches;

                // Detect features on warped image
                extractFeatures(m_warpedImg, warpedKeypoints, m_queryDescriptors);

                // Match with pattern
                getMatches(m_queryDescriptors, refinedMatches);

                // Estimate new refinement homography
                homographyFound = refineMatchesWithHomography(//这里是计算经过之前计算的单应性矩阵反变换后的透视图像与模板图案的单应性矩阵，得到匹配组合
                        warpedKeypoints,
                        m_pattern.keypoints,
                        homographyReprojectionThreshold,
                        refinedMatches,
                        m_refinedHomography);

                // Get a result homography as result of matrix product of refined and rough homographies:
                info.homography =
                        m_roughHomography * m_refinedHomography;//得到最终的单应性矩阵，是第一次计算的单应性矩阵与第二次得到的相乘。

                // Transform contour with rough homography
#if _DEBUG
                cv::perspectiveTransform(m_pattern.points2d, info.points2d, m_roughHomography);//使用第一次计算的单应性矩阵对模板图像的四个顶点变换成目标图像的四个顶点。
                info.draw2dContour(tmp, CV_RGB(0,200,0));//用直线把这四个顶点连起来。
#endif

                // Transform contour with precise homography
                cv::perspectiveTransform(m_pattern.points2d, info.points2d,
                                         info.homography);//使用优化后的单应性矩阵进行转换。在测试图像上得到用优化的单应性矩阵计算出的四个顶点的二维坐标。
                //#if _DEBUG
                info.draw2dContour(tmp, CV_RGB(200, 0, 0));
                //#endif
            }
            else  //不进行二次变换就一次变换。
            {
                info.homography = m_roughHomography;

                // Transform contour with rough homography
                cv::perspectiveTransform(m_pattern.points2d, info.points2d, m_roughHomography);
#if _DEBUG
                info.draw2dContour(tmp, CV_RGB(0,200,0));
#endif

            }

        }

        //这个还是第一次变换后的匹配，不过程序不退出就一直运行，得到的图像也几乎不变。
        // cv::showAndSave("Final matches", getMatchesImage(tmp, m_pattern.frame, m_queryKeypoints, m_pattern.keypoints, m_matches, 100));
        matched = getMatchesImage(tmp, m_pattern.frame, m_queryKeypoints, m_pattern.keypoints,
                                  m_matches, 100);

        std::cout << "Features:" << std::setw(4) << m_queryKeypoints.size() << " Matches: " <<
        std::setw(4) << m_matches.size() << std::endl;

        return homographyFound;
    }

    void PatternDetector::getGray(const cv::Mat &image, cv::Mat &gray) {
        if (image.channels() == 3)
            cv::cvtColor(image, gray, CV_BGR2GRAY);
        else if (image.channels() == 4)
            cv::cvtColor(image, gray, CV_BGRA2GRAY);
        else if (image.channels() == 1)
            gray = image;
    }


    //这个就是根据给定的查询描述子集，之前计算出的训练描述子集，对每一个查询描述子集，找到一个最佳的训练描述子，并把找到的匹配放在容器里，就是二者的下标，匹配距离等一些参数。
    /*OpenCV提供了两种Matching方式：
    • Brute-force matcher (cv::BFMatcher)
    • Flann-based matcher (cv::FlannBasedMatcher)
    Brute-force matcher就是用暴力方法找到点集一中每个descriptor在点集二中距离最近的descriptor；
    Flann-based matcher 使用快速近似最近邻搜索算法寻找（用快速的第三方库近似最近邻搜索算法）
    一般把点集一称为 train set （训练集）对应模板图像，点集二称为 query set（查询集）对应查找模板图的目标图像。
    为了提高检测速度，你可以调用matching函数前，先训练一个matcher。训练过程可以首先使用cv::FlannBasedMatcher来优化，为descriptor建立索引树，这种操作将在匹配大量数据时发挥巨大作用（比如在上百幅图像的数据集中查找匹配图像）。而Brute-force matcher在这个过程并不进行操作，它只是将train descriptors保存在内存中。*/
    void PatternDetector::getMatches(const cv::Mat &queryDescriptors,
                                     std::vector<cv::DMatch> &matches)//DMatch用于匹配特征关键点的特征描述子的类：查询特征描述子索引, 特征描述子索引, 训练图像索引, 以及不同特征描述子之间的距离.
    {
        matches.clear();

        if (enableRatioTest) {
            // To avoid NaN's when best match has zero distance we will use inversed ratio.
            ////为了避免NaN's，当最好的匹配是零距离时，我们将使用相反的比例
            const float minRatio = 1.f / 1.5f;

            // KNN match will return 2 nearest matches for each query descriptor给定查询集合中的每个特征描述子，寻找K个最佳匹配。
            //使用KNN-matching算法，令K=2。则每个match得到两个最接近的descriptor，然后计算最接近距离和次接近距离之间的比值，当比值大于既定值时，才作为最终match。
            m_matcher->knnMatch(queryDescriptors, m_knnMatches, 2);

            for (size_t i = 0; i < m_knnMatches.size(); i++) {
                const cv::DMatch &bestMatch = m_knnMatches[i][0];//最佳匹配
                const cv::DMatch &betterMatch = m_knnMatches[i][1];//次佳匹配

                float distanceRatio = bestMatch.distance / betterMatch.distance;//计算这俩的比值

                // Pass only matches where distance ratio between
                // nearest matches is greater than 1.5 (distinct criteria)
                if (distanceRatio < minRatio)//如果比值小于最小比率，则把那个最佳匹配放在匹配集合里返回。
                {
                    matches.push_back(bestMatch);
                }
            }
        }
        else {
            // Perform regular match//这个就是寻找最佳匹配，应该是用的初始化里的匹配方法。
            m_matcher->match(queryDescriptors, matches);
        }
    }

    //为了进一步提升匹配精度，可以采用随机样本一致性（RANSAC）方法
    //因为我们是使用一幅图像（一个平面物体），我们可以将它定义为刚性的，可以在pattern image和query image的特征点之间找到单应性变换（homography transformation）。使用cv::findHomography找到这个单应性变换，使用RANSAC找到最佳单应性矩阵。（由于这个函数使用的特征点同时包含正确和错误匹配点，因此计算的单应性矩阵依赖于二次投影的准确性）
    //利用findHomography函数利用匹配的关键点找出相应的变换，再利用perspectiveTransform函数映射点群。
    bool PatternDetector::refineMatchesWithHomography
            (
                    const std::vector<cv::KeyPoint> &queryKeypoints,
                    const std::vector<cv::KeyPoint> &trainKeypoints,
                    float reprojectionThreshold,
                    std::vector<cv::DMatch> &matches,
                    cv::Mat &homography
            ) {
        const int minNumberMatchesAllowed = 8;//最少有8组匹配。

        if (matches.size() < minNumberMatchesAllowed)
            return false;

        // Prepare data for cv::findHomography
        std::vector<cv::Point2f> srcPoints(matches.size());
        std::vector<cv::Point2f> dstPoints(matches.size());

        for (size_t i = 0; i < matches.size(); i++) {
            srcPoints[i] = trainKeypoints[matches[i].trainIdx].pt;//.pt是得到二维坐标
            dstPoints[i] = queryKeypoints[matches[i].queryIdx].pt;
        }

        // Find homography matrix and get inliers mask
        std::vector<unsigned char> inliersMask(srcPoints.size());
        homography = cv::findHomography(
                srcPoints, //寻找匹配上的关键点的单应性变换，求两幅图像的单应性矩阵，它是一个3*3的矩阵，但这里的单应性矩阵和3D重建中的单应性矩阵（透视矩阵3*4）是不一样的。之前一直混淆了两者的区别。
                dstPoints, //前两个参数两幅图像匹配的点
                CV_FM_RANSAC, //计算单应性矩阵使用方法
                reprojectionThreshold, //允许的最大反投影错误，在使用RANSAC时才有。
                inliersMask);//指出匹配的值是不是离群值，用来优化匹配结果。这是一个向量，大小和匹配点个数相同，对每一组匹配判断是不是离群值，是这个元素就为0.

        std::vector<cv::DMatch> inliers;
        for (size_t i = 0; i < inliersMask.size(); i++) {
            if (inliersMask[i])
                inliers.push_back(matches[i]);
        }

        matches.swap(inliers);
        return matches.size() > minNumberMatchesAllowed;//若有八组以上匹配，那么认为找到这些匹配的单应性矩阵。
    }

}//end of namespace ml_ar
