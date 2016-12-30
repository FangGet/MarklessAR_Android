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
#include "ARDrawingContext.hpp"

////////////////////////////////////////////////////////////////////
// Standard includes:
#include<EGL/egl.h>
#include <GLES/gl.h>
#include<GLES2/gl2.h>
#include<GLES2/gl2ext.h>

namespace ml_ar {

    //在OpenGL的初始化中，调用namedWindow使用最后一个参数调用cv::WINDOW_OPENGL，把OpenGL接口到highgui的模块中，设置窗口大小。
    //然后调用setOpenGLContext建立窗口关联（参数就是窗口名称），为了在这个窗口上画虚拟物体，需要使用回调函数，建立方法就是setOpenGLDrawCallback，
    //注意这个函数第一个参数是窗口名称，第二个参数是回调函数名，第三个参数是回调函数的参数，因为我这里回调函数onDraw是无参函数，所以这里为NULL
    //在需要重绘的时候还要调用updateWindow
    ARDrawingContext::ARDrawingContext(cv::Size frameSize, const CameraCalibration &c)
            : m_isTextureInitialized(false), m_calibration(c) {
    }

    ARDrawingContext::~ARDrawingContext() {
    }

    void ARDrawingContext::updateBackground(const cv::Mat &frame) {
        frame.copyTo(m_backgroundImage);//复制原图像给成员变量，作为背景。
    }

    void ARDrawingContext::updateWindow() {
        draw();
    }

    void ARDrawingContext::draw() {
        glClear(GL_DEPTH_BUFFER_BIT |
                GL_COLOR_BUFFER_BIT); // Clear entire screen:清除屏幕，这在OpenGL绘制前必须要干的事

        drawCameraFrame();                                  // Render background，渲染背景
        drawAugmentedScene();                               // Draw AR
        glFlush();
    }


    void ARDrawingContext::drawCameraFrame() {
        // Initialize texture for background image
        if (!m_isTextureInitialized) {
            glGenTextures(1,
                          &m_backgroundTextureId);//1：用来生成纹理的数量，&m_backgroundTextureId存储纹理索引的第一个元素指针，glDeleteTextures,用于销毁一个纹理
            glBindTexture(GL_TEXTURE_2D,
                          m_backgroundTextureId);//实际上是改变了OpenGL的这个状态，它告诉OpenGL下面对纹理的任何操作都是对它所绑定的纹理对象的，比如   glBindTexture(GL_TEXTURE_2D,1)告诉OpenGL下面代码中对2D纹理的任何设置都是针对索引为1的纹理的。

            //图象从纹理图象空间映射到帧缓冲图象空间(映射需要重新构造纹理图像,这样就会造成应用到多边形上的图像失真),这时就可用glTexParmeteri()函数来确定如何把纹理象素映射成像素.这里是对2D纹理先进行缩小线性过滤，然后再进行放大线性过滤（使用距离当前渲染像素中心最近的4个纹素加权平均值）。
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//纹理过滤函数
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            m_isTextureInitialized = true;//这个是纹理是否已经进行初始化的标志。
        }

        int w = m_backgroundImage.cols;//获取行列值
        int h = m_backgroundImage.rows;

        glPixelStorei(GL_PACK_ALIGNMENT,
                      1);//设置像素存储模式，GL_PACK_ALIGNMENT，它影响将像素数据写回到主存的打包形式，对glReadPixels的调用产生影响；指定相应的pname设置为什么值，用于指定存储器中每个像素行有多少个字节对齐。对齐的字节数越高，系统就越能优化。
        glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);//下面继续对索引为1的纹理进行处理。

        // Upload new texture data:
        if (m_backgroundImage.channels() == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE,
                         m_backgroundImage.data);//根据指定的参数，生成一个2D纹理（Texture）。
        else if (m_backgroundImage.channels() == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         m_backgroundImage.data);
        else if (m_backgroundImage.channels() == 1)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         m_backgroundImage.data);

        const GLfloat bgTextureVertices[] = {0, 0, (GLfloat) w, 0, 0, (GLfloat) h, (GLfloat) w,
                                             (GLfloat) h};
        const GLfloat bgTextureCoords[] = {1, 0, 1, 1, 0, 0, 0, 1};
        const GLfloat proj[] = {0, -2.f / w, 0, 0, -2.f / h, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1};

        glMatrixMode(GL_PROJECTION);//导入投影矩阵
        glLoadMatrixf(proj);

        glMatrixMode(GL_MODELVIEW);//导入视景矩阵
        glLoadIdentity();

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_backgroundTextureId);

        // Update attribute values.
        glEnableClientState(GL_VERTEX_ARRAY);//开启顶点数组功能
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);//这个应该是纹理。

        glVertexPointer(2, GL_FLOAT, 0, bgTextureVertices);//用一个数组指定了每个顶点的坐标
        glTexCoordPointer(2, GL_FLOAT, 0, bgTextureCoords);//开启纹理属性

        glColor4f(1, 1, 1, 1);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);//当采用顶点数组方式绘制图形时，使用该函数。该函数根据顶点数组中的坐标数据和指定的模式，进行绘制。

        glDisableClientState(GL_VERTEX_ARRAY);//关闭顶点数组功能
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);//关闭纹理数组
        glDisable(GL_TEXTURE_2D);//关闭2D纹理
    }

    void ARDrawingContext::drawAugmentedScene() {
        // Init augmentation projection
        Matrix44 projectionMatrix;
        int w = m_backgroundImage.cols;
        int h = m_backgroundImage.rows;
        buildProjectionMatrix(m_calibration, w, h, projectionMatrix);

        glMatrixMode(GL_PROJECTION);//加载投影矩阵
        glLoadMatrixf(projectionMatrix.data);

        glMatrixMode(GL_MODELVIEW);//加载视景矩阵。
        glLoadIdentity();

        if (isPatternPresent) {
            // Set the pattern transformation
            Matrix44 glMatrix = patternPose.getMat44();
            glLoadMatrixf(reinterpret_cast<const GLfloat *>(&glMatrix.data[0]));

            // Render model
            drawCoordinateAxis();//绘制坐标轴
            drawCubeModel();//绘制一定透明度的蓝色四方体
        }
    }

    //根据传进来的相机校正参数建立投影矩阵
    void ARDrawingContext::buildProjectionMatrix(const CameraCalibration &calibration,
                                                 int screen_width, int screen_height,
                                                 Matrix44 &projectionMatrix) {
        float nearPlane = 0.01f;  // Near clipping distance
        float farPlane = 100.0f;  // Far clipping distance

        // Camera parameters
        float f_x = calibration.fx(); // Focal length in x axis
        float f_y = calibration.fy(); // Focal length in y axis (usually the same?)
        float c_x = calibration.cx(); // Camera primary point x
        float c_y = calibration.cy(); // Camera primary point y

        projectionMatrix.data[0] = -2.0f * f_x / screen_width;
        projectionMatrix.data[1] = 0.0f;
        projectionMatrix.data[2] = 0.0f;
        projectionMatrix.data[3] = 0.0f;

        projectionMatrix.data[4] = 0.0f;
        projectionMatrix.data[5] = 2.0f * f_y / screen_height;
        projectionMatrix.data[6] = 0.0f;
        projectionMatrix.data[7] = 0.0f;

        projectionMatrix.data[8] = 2.0f * c_x / screen_width - 1.0f;
        projectionMatrix.data[9] = 2.0f * c_y / screen_height - 1.0f;
        projectionMatrix.data[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        projectionMatrix.data[11] = -1.0f;

        projectionMatrix.data[12] = 0.0f;
        projectionMatrix.data[13] = 0.0f;
        projectionMatrix.data[14] = -2.0f * farPlane * nearPlane / (farPlane - nearPlane);
        projectionMatrix.data[15] = 0.0f;
    }


    void ARDrawingContext::drawCoordinateAxis()//绘制坐标轴，颜色各不同。
    {
        glLineWidth(3.f);
        glEnable(GL_COLOR_MATERIAL);
        glEnableClientState(GL_VERTEX_ARRAY);
        glColor4f(1.0f, 0.0f, 0.0f, 1.f);
        GLfloat lineX[] = {0, 0, 0, 1, 0, 0};
        glVertexPointer(3, GL_FLOAT, 0, lineX);
        glDrawArrays(GL_LINES, 0, 2);

        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
        GLfloat lineY[] = {0, 0, 0, 0, 1, 0};
        glVertexPointer(3, GL_FLOAT, 0, lineY);
        glDrawArrays(GL_LINES, 0, 2);

        glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
        GLfloat lineZ[] = {0, 0, 0, 0, 0, 1};
        glVertexPointer(3, GL_FLOAT, 0, lineZ);
        glDrawArrays(GL_LINES, 0, 2);

        glDisableClientState(GL_VERTEX_ARRAY);
    }

    void ARDrawingContext::drawCubeModel() {
        static const GLfloat LightAmbient[] = {0.25f, 0.25f, 0.25f,
                                               1.0f};    // Ambient Light Values
        static const GLfloat LightDiffuse[] = {0.1f, 0.1f, 0.1f, 1.0f};    // Diffuse Light Values
        static const GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};    // Light Position


        glColor4f(0.2f, 0.35f, 0.3f, 0.75f);         // Full Brightness, 50% Alpha ( NEW )
        glBlendFunc(GL_ONE,
                    GL_ONE_MINUS_SRC_ALPHA);       // Blending Function For Translucency Based On Source Alpha
        glEnable(GL_BLEND);

        glShadeModel(GL_SMOOTH);

        glEnable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
        glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
        glEnable(GL_COLOR_MATERIAL);

        glScalef(0.25, 0.25, 0.25);
        glTranslatef(0, 0, 1);

        const GLfloat l = -0.5f;
        const GLfloat h = 0.5f;

        const GLfloat verts[] = {
                l, l, h, h, l, h, l, h, h, h, h, h,  // FRONT
                l, l, l, l, h, l, h, l, l, h, h, l,  // BACK
                l, l, h, l, h, h, l, l, l, l, h, l,  // LEFT
                h, l, l, h, h, l, h, l, h, h, h, h,  // RIGHT
                l, h, h, h, h, h, l, h, l, h, h, l,  // TOP
                l, l, h, l, l, l, h, l, h, h, l, l   // BOTTOM
        };

        glVertexPointer(3, GL_FLOAT, 0, verts);
        glEnableClientState(GL_VERTEX_ARRAY);

        glColor4f(1.0f, 0.0f, 0.0f, 0.7f);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

        glColor4f(0.0f, 1.0f, 0.0f, 0.7f);
        glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);

        glColor4f(0.0f, 0.0f, 1.0f, 0.7f);
        glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
    }
}//namespace ml_ar
