#pragma once

#include <QtCore>
#include <QtGui>
#include <QApplication>
#include <QtOpenGL>
#include <QGLWidget>
#include <QMainWindow>
#include <QOpenGLShaderProgram>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


class frag_window;

class frag_pane : public QGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
       
public:
   // image processing
   enum {
      // Image is not Selected
      ImageProcInvalid = 0, 
      ImageProcOriginal = 1, 
      ImageProcInverse = 101,
      ImageProcMosaic = 201,
      ImageProcMedian3X3 = 403,
      ImageProcLResample = 523
   } ImageProcessingMethod;

private:            
   // pointer to the main window
   frag_window *_pParent;
   
   // one of ImageProcessingMethod
   short _sImageProc;
      
private:
   // shader programs
    
   QOpenGLShaderProgram _copy_program;
   QOpenGLShaderProgram _inverse_program;
   QOpenGLShaderProgram _mosaic_program;
   QOpenGLShaderProgram _median3_program;
   QOpenGLShaderProgram _lresample_program;

   // FBO that contains the texture(image) read from the file
   QGLFramebufferObject *_in_fbo;
   
   // FBO that carries the texture to be shown on the pane
   QGLFramebufferObject *_out_fbo; 
public slots:
    bool frag_pane::showImage( cv::Mat image );
    //bool frag_pane::takeImage( cv::Mat image );

public:
   explicit frag_pane(QWidget *parent = 0);
   ~frag_pane();  
   
   void clear( void );
   bool setTexture( void );
   bool loadImage( const QString strFile );
   bool loadPic( cv::Mat pic);
   bool loadFrame();
   bool saveImage( const QString strFile );

   void setParent( frag_window *pMain ) { _pParent = pMain; }
   frag_window *getParent( void ) { return _pParent; }
   
   void refreshImage( void );
   void bindShader( void );
   void releaseShader( void );
   
   bool validImageProc( void ) const;
   
   void resetImageProc( void ) { _sImageProc = ImageProcInvalid; }
   void setImageProc( short sImageProc ) { _sImageProc = sImageProc; }
   short getImageProc( void ) const { return _sImageProc; }



protected:
   void initializeGL( void );
   void paintGL( void );
   void resizeGL(int w, int h);
   void renderImage( void );

void        updateScene();
   
private:
   bool createShaders( void );
   bool createInverseShader( void );
      
   bool createSingleShader( QOpenGLShaderProgram *pShader,
      const QString strVertexFile, 
      const QString strFragmentFile );
private:
      bool        mSceneChanged;

   QImage      mRenderQtImg;           /// Qt image to be rendered
   cv::Mat     mOrigImage;             /// original OpenCV image to be shown


   QColor      mBgColor;		/// Background color

   int         mOutH;                  /// Resized Image height
   int         mOutW;                  /// Resized Image width
   float       mImgRatio;             /// height/width ratio

   int         mPosX;                  /// Top left X position to render image in the center of widget
   int         mPosY;                  /// Top left Y position to render image in the center of widget

signals:
    void    imageSizeChanged( int outW, int outH ); /// Used to resize the image outside the widget

};

