#ifndef CVQTVIEWER
#define CVQTVIEWER

#include <QTime>
#include <QtCore>
#include <QScreen>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QDesktopWidget>
#include <QColor>
#include <QApplication>
#include <QGuiApplication>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLVertexArrayObject>
#include <QGLFormat>
#include <QOpenGLWidget>
#include <QGLWidget>
#include <algorithm>
#include <math.h>
#include <opencv2/core/core.hpp>



class cvQtViewer : public QGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit cvQtViewer(QWidget *parent = 0);
    ~cvQtViewer();

signals:
    void    imageSizeChanged( int outW, int outH ); /// Used to resize the image outside the widget

public slots:
    bool    showImage( cv::Mat image ); /// Used to set the image to be viewed

protected:
    void 	initializeGL(); /// OpenGL initialization
    void 	paintGL(); /// OpenGL Rendering
    void 	resizeGL(int width, int height);        /// Widget Resize Event

    void        updateScene();
    void        renderImage();
    void        initShaders();
    void        bindShader();
    void        releaseShader();

private:
  // enum VAO_IDs { Triangles, NumVAOs };

    bool        mSceneChanged;          /// Indicates when OpenGL view is to be redrawn

   // GLuint      VAOs[NumVAOs];
  //  const GLuint NumVertices = 6;
    QImage      mRenderQtImg;           /// Qt image to be rendered
    cv::Mat     mOrigImage;             /// original OpenCV image to be shown

    QColor      mBgColor;		/// Background color

    int         mOutH;                  /// Resized Image height
    int         mOutW;                  /// Resized Image width
    float       mImgRatio;             /// height/width ratio

    int         mPosX;                  /// Top left X position to render image in the center of widget
    int         mPosY;                  /// Top left Y position to render image in the center of widget

    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLShaderProgram program;
    QOpenGLTexture *texture;
    QOpenGLBuffer indiceBuffer, vertexBuffer;

    GLuint tex;
};


#endif // CVQTVIEWER

