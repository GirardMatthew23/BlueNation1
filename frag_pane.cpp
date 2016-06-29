
#include <QtCore>
#include <QtGui>
#include <QApplication>
#include <QtOpenGL>
#include <QGLWidget>
#include <QMainWindow>
#include <QOpenGLShaderProgram>
#include <QTime>

#include <vector>
#include <string>
#include <algorithm>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "frag_window.h"
#include "frag_pane.h"

frag_pane::frag_pane(QWidget *parent)
    : QGLWidget(parent)
{
   _in_fbo = NULL;
   _out_fbo = NULL;

   clear( );


   mOutH = 0;
   mOutW = 0;
   mImgRatio = 4.0f/3.0f;

   mPosX = 0;
   mPosY = 0;

   return;
}

frag_pane::~frag_pane()
{
   if( _in_fbo ) {
      delete _in_fbo;
      _in_fbo = NULL;
   }

   if( _out_fbo ) {
      delete _out_fbo;
      _out_fbo = NULL;
   }

   return;
}

bool frag_pane::validImageProc( void )
   const
{
   bool bRetCode = false;

   switch( _sImageProc ) {
   case ImageProcOriginal:
   case ImageProcInverse:
   case ImageProcMosaic:
   case ImageProcMedian3X3:
   case ImageProcLResample:
      break;

   case ImageProcInvalid:
   default:
      goto PIX_EXIT;
      break;
   }

   // --- DONE ---
   bRetCode = true;
PIX_EXIT:
   return bRetCode;
}

void frag_pane::clear( void )
{
   _sImageProc = ImageProcInvalid;

   return;
}

void frag_pane::initializeGL( void )
{
   initializeOpenGLFunctions( );
   glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

   // create all shader programs

   if( !createShaders( ) ) {
      qDebug() << "ERROR! Failed to create shader programs.";
      goto PIX_EXIT;
   }

PIX_EXIT:
   return;
}
/*
void frag_pane::initializeGL()
{
    makeCurrent();
    initializeOpenGLFunctions();

    // SET THE BACKGROUND COLOR TO PUKE GREEN
    glClearColor(0.0f,0.0f,0.0f,1.0f);

//    initShaders();
 //   bindShader();

}*/


void frag_pane::paintGL( void )
{
   if( !validImageProc( ) ) {
      goto PIX_EXIT;
   }

   refreshImage( );

PIX_EXIT:
   return;
}

/*
void frag_pane::paintGL()
{
   // glClear(GL_COLOR_BUFFER_BIT);


    if( !mSceneChanged )
        return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderImage();


    mSceneChanged = false;
}*/

void frag_pane::resizeGL( int width,
                            int height )
{
    makeCurrent();
    glViewport(0, 0, (GLint)width, (GLint)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, width, 0, height, 0, 1);	// To Draw image in the center of the area

    glMatrixMode(GL_MODELVIEW);

    // ---> Scaled Image Sizes
    mOutH = width/mImgRatio;
    mOutW = width;

    if(mOutH>height)
    {
        mOutW = height*mImgRatio;
        mOutH = height;
    }

    emit imageSizeChanged( mOutW, mOutH );
    // <--- Scaled Image Sizes

    mPosX = (width-mOutW)/2;
    mPosY = (height-mOutH)/2;

    mSceneChanged = true;

    refreshImage();

    updateScene();
}

//
// create all shader programs
//
bool frag_pane::createShaders( void )
{
   bool bRetCode = false;

   // original image

   if( !createSingleShader( &_copy_program,
      ":/shaders/copy.vert",
      ":/shaders/copy.frag" ) ) {
         qDebug() << "ERROR! Failed to Create Copy Shader.";
         goto PIX_EXIT;
   }

   // inverse(negative) image

   if( !createInverseShader( ) ) {
      qDebug() << "ERROR! Failed to Create Negative Shader.";
      goto PIX_EXIT;
   }

   // mosaic

   if( !createSingleShader( &_mosaic_program,
      ":/shaders/mosaic.vert",
      ":/shaders/mosaic.frag" ) ) {
         qDebug() << "ERROR! Failed to Create Mosaic Shader.";
         goto PIX_EXIT;
   }

   // median filter

   if( !createSingleShader( &_median3_program,
      ":/shaders/median3.vert",
      ":/shaders/median3.frag" ) ) {
         qDebug() << "ERROR! Failed to Create Median Shader.";
         goto PIX_EXIT;
   }

   //LResample
   // median filter

   if( !createSingleShader( &_lresample_program,
      ":/shaders/lresample.vert",
      ":/shaders/lresample.frag" ) ) {
         qDebug() << "ERROR! Failed to Create LResample Shader.";
         goto PIX_EXIT;
   }

   // --- DONE ---
   bRetCode = true;
PIX_EXIT:
   return bRetCode;
}

//
// show the image from upper left corner
//
void frag_pane::refreshImage( void )
{
   int nPaneWidth = size( ).width( );
   int nPaneHeight = size( ).height( );
   int nImgWidth, nImgHeight;

   if( !validImageProc( ) ) {
      goto PIX_EXIT;
   }

   if( !_out_fbo ) {
      goto PIX_EXIT;
   }

   // *** draw the texture in FBO on pane ***

   nImgWidth = _out_fbo->size().width( );
   nImgHeight = _out_fbo->size().height( );

   makeCurrent( );

   glViewport( 0, 0, nPaneWidth, nPaneHeight );
   glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
   glClear( GL_COLOR_BUFFER_BIT );

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

    glOrtho( 0.0, nPaneWidth, 0.0, nPaneHeight, -1.0, 1.0 );

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity( );

   glEnable( GL_TEXTURE_2D );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

   glBindTexture( GL_TEXTURE_2D, _out_fbo->texture( ) );

   glBegin( GL_QUADS );
      glTexCoord2d( 0.0, 0.0 );
       glVertex2i( 0, 0 );
      glTexCoord2d( 1.0, 0.0 );
       glVertex2i( nImgWidth, 0 );
      glTexCoord2d( 1.0, 1.0 );
       glVertex2i( nImgWidth, nImgHeight );
      glTexCoord2d( 0.0, 1.0 );
       glVertex2i( 0, nImgHeight );
    glEnd();

   glFlush( );
   swapBuffers( );

PIX_EXIT:
   return;
}

//
// set the image texture into FBO
//
bool frag_pane::setTexture( void )
{
   bool bRetCode = false;
   QImageReader reader;
   int nWidth, nHeight;

   // *** Create FBO ***

   if( !_in_fbo ) {
      // image is not read yet
      goto PIX_EXIT;
   }

   nWidth = _in_fbo->size( ).width( );
   nHeight = _in_fbo->size( ).height( );

   // create(or re-create) the framebuffer object - make sure to have a current
   // context before creating it

   if( _out_fbo ) {
      delete _out_fbo;
      _out_fbo = NULL;
   }

   makeCurrent( );
   _out_fbo = new QGLFramebufferObject( nWidth, nHeight, GL_TEXTURE_2D );
   if( !_out_fbo ) {
      goto PIX_EXIT;
   }

   // set the texture on FBO

   bindShader( );

   _out_fbo->bind( );

   glViewport( 0, 0, nWidth, nHeight );

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();

   glOrtho( 0.0, nWidth, 0.0, nHeight, -1.0, 1.0 );

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity( );

   glEnable( GL_TEXTURE_2D );

   _out_fbo->drawTexture( QPointF(0.0,0.0), _in_fbo->texture( ), GL_TEXTURE_2D );

   releaseShader( );

   _out_fbo->release();

   // --- DONE ---
   bRetCode = true;
PIX_EXIT:
   return bRetCode;
}

//
// bind the shader and set uniform values
//
void frag_pane::bindShader( void )
{
   int nWidth, nHeight;

   if( !_out_fbo ) {
      goto PIX_EXIT;
   }

   nWidth = _out_fbo->size().width();
   nHeight = _out_fbo->size().height();
   QTime nTime;

   switch( _sImageProc ) {

   case ImageProcOriginal:
      qDebug() << "....Original Image";
      _copy_program.bind( );
      _copy_program.setUniformValue( "image", 0 );
      _copy_program.setUniformValue( "imgWidth", nWidth );
      _copy_program.setUniformValue( "imgHeight", nHeight );
      break;

   case ImageProcInverse:
      qDebug() << "....Color Inverse Filter";
      _inverse_program.bind( );
      _inverse_program.setUniformValue( "image", 0 );
      _inverse_program.setUniformValue( "imgWidth", nWidth );
      _inverse_program.setUniformValue( "imgHeight", nHeight );
      break;

   case ImageProcMosaic:
      qDebug() << "....Mosaic Filter";
      _mosaic_program.bind( );
      _mosaic_program.setUniformValue( "image", 0 );
      _mosaic_program.setUniformValue( "imgWidth", nWidth );
      _mosaic_program.setUniformValue( "imgHeight", nHeight );
      _mosaic_program.setUniformValue( "nPixels", 12 );
      break;

   case ImageProcMedian3X3:
      qDebug() << "....Median Filter 3X3";
      _median3_program.bind( );
      _median3_program.setUniformValue( "image", 0 );
      _median3_program.setUniformValue( "imgWidth", nWidth );
      _median3_program.setUniformValue( "imgHeight", nHeight );
      break;

   case ImageProcLResample:
      qDebug() << "....Lau's Resample";
      _lresample_program.bind( );
      _lresample_program.setUniformValue( "image", 0 );
      _lresample_program.setUniformValue( "imgWidth", nWidth );
      _lresample_program.setUniformValue( "imgHeight", nHeight );
     // _lresample_program.setUniformValue( "nPixels", 15 );
      break;

   default:
      qDebug() << "....Unknown Filter " << _sImageProc;
      break;
   }

PIX_EXIT:
   return;
}

//
// release the shader program after the image in shown
//
void frag_pane::releaseShader( void )
{
   // unlink the shader

   switch( _sImageProc ) {
   case ImageProcOriginal:
      _copy_program.release( );
      break;

   case ImageProcInverse:
      _inverse_program.release( );
      break;

   case ImageProcMosaic:
      _mosaic_program.release( );
      break;

   case ImageProcMedian3X3:
      _median3_program.release( );
      break;

   case ImageProcLResample:
      _lresample_program.release( );
      break;
   }

   return;
}

//
// load image and store into opengl texture
// memorize image size
//
bool frag_pane::loadImage( const QString strFile )
{
   bool bRetCode = false;
   QImage img;
   QRgb value;
   QImageReader reader;
   int k, iRow, i, j;
   int nWidth, nHeight;

#ifdef _DEBUG
   QImage img2;
#endif // _DEBUG

   GLubyte *image = NULL;

   // *** Load Image ***

   // load image using QImageReader
   // set image pixels to one-dimensional 8-bit RGBA buffer

   reader.setFileName( strFile );
   if( !reader.canRead( ) ) {
      qDebug() << "ERROR! Failed to load image " << strFile;
      goto PIX_EXIT;
    }

   if( !reader.read( &img ) ) {
      qDebug() << "ERROR! Failed to read image " << strFile;
      goto PIX_EXIT;
   }

    nWidth = img.width( );
    nHeight = img.height( );

   image = new GLubyte[nHeight*nWidth*4];
   if( !image ) {
      qDebug() << "ERROR! Memory Allocation Failed.";
      goto PIX_EXIT;
   }

   // to display the texture directory onto pane, set the buffer upside down
   // to copy on FBO, use the image orientation as it is

   for( i = 0; i < nHeight; i++ ) {
      iRow = nHeight - i - 1;
      //iRow = i;
      for( j = 0 ; j < nWidth ; j++ ) {
         k = i * nWidth + j;
         value = img.pixel( j, iRow );
         image[k*4] = qRed( value );
         image[k*4+1] = qGreen( value );
         image[k*4+2] = qBlue( value );
         image[k*4+3] = 255;
      }
   }

   // create(or re-create) the FBO to set the loaded image

   if( _in_fbo ) {
      delete _in_fbo;
      _in_fbo = NULL;
   }

   makeCurrent( );
   _in_fbo = new QGLFramebufferObject( nWidth, nHeight, GL_TEXTURE_2D );
   if( !_in_fbo ) {
      goto PIX_EXIT;
   }

   // set the buffer into texture

   glEnable( GL_TEXTURE_2D );

   _in_fbo->bind( );

   glEnable( GL_TEXTURE_2D );
   glViewport( 0, 0, nWidth, nHeight );

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

    glOrtho( 0.0, nWidth, 0.0, nHeight, -1.0, 1.0 );

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity( );

   glBindTexture( GL_TEXTURE_2D, _in_fbo->texture( ) );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

   glTexImage2D( GL_TEXTURE_2D,
      0,
      GL_RGBA,
      nWidth,
      nHeight,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      image );

   _in_fbo->release();

   // --- DONE ---
   bRetCode = true;
PIX_EXIT:
   if( image ) {
      delete[] image;
      image = NULL;
   }
   return bRetCode;
}

//
// save image in the opengl texture into file
//
bool frag_pane::saveImage( const QString strFile )
{
   bool bRetCode = false;
   QImageWriter writer;
   int nWidth, nHeight;
   int k, iRow, i, j, iRed, iGreen, iBlue;
   QImage *img = NULL;
   GLubyte *image = NULL;

   if( !validImageProc( ) ) {
      goto PIX_EXIT;
   }

   if( !_out_fbo ) {
      goto PIX_EXIT;
   }

   nWidth = _out_fbo->size().width( );
   nHeight = _out_fbo->size().height( );

   // get the buffer into texture

   _out_fbo->bind( );

   glEnable( GL_TEXTURE_2D );
   glViewport( 0, 0, nWidth, nHeight );

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

    glOrtho( 0.0, nWidth, 0.0, nHeight, -1.0, 1.0 );

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity( );

   glBindTexture( GL_TEXTURE_2D, _out_fbo->texture( ) );

   image = new GLubyte[nHeight*nWidth*4];
   if( !image ) {
      qDebug() << "ERROR! Memory Allocation Failed.";
      goto PIX_EXIT;
   }

   glReadPixels( 0, 0, nWidth, nHeight, GL_RGB, GL_UNSIGNED_BYTE,
      image );

   // *** create image ***

   img = new QImage( nWidth, nHeight, QImage::Format_RGB888 );
   if( !img ) {
      qDebug() << "ERROR! Failed to Create QImage.";
      goto PIX_EXIT;
   }

   for( i = 0; i < nHeight; i++ ) {
      iRow = nHeight - i - 1;
      for( j = 0 ; j < nWidth ; j++ ) {
         k = i * nWidth + j;
         iRed = (int)image[k*3];
         iGreen = (int)image[k*3+1];
         iBlue = (int)image[k*3+2];
         img->setPixel( j, iRow, qRgb( iRed, iGreen, iBlue ) );
      }
   }

   // save the image

   writer.setFileName( strFile );
   if( !writer.canWrite( ) ) {
      qDebug() << "ERROR! Can't write to this file " << strFile;
      goto PIX_EXIT;
    }

   if( !writer.write( *img ) ) {
      qDebug() << "ERROR! Failed to read image " << strFile;
      goto PIX_EXIT;
   }

   _out_fbo->release( );

   // --- DONE ---
   bRetCode = true;
PIX_EXIT:
   if( image ) {
      delete[] image;
      image = NULL;
   }
   if( img ) {
      delete img;
      img = NULL;
   }
   return bRetCode;
}

//
// create inver filter
// shader programs are embeded in the source file
//
bool frag_pane::createInverseShader( void )
{
   bool bRetCode = false;
   QOpenGLShader *vshader1 = NULL;
   QOpenGLShader *fshader1 = NULL;

   const char *vsrc1 =
      "varying vec2 pos;\n"
      "void main( void )\n"
      "{\n"
      "	pos = gl_Vertex.xy;\n"
      "	gl_Position = ftransform( );\n"
      "}\n";

   const char *fsrc1 =
      "uniform sampler2D image;\n"
      "uniform int imgWidth;\n"
      "uniform int imgHeight;\n"
      "varying vec2 pos;\n"
      "void main(void)\n"
      "{\n"
      "   vec2 texCoord = vec2( pos.x / float( imgWidth ), pos.y / float( imgHeight ) );\n"
      "	vec4 col = texture2D( image, texCoord );\n"
      "	gl_FragColor.r = 1.0 - col.r;\n"
      "	gl_FragColor.g = 1.0 - col.g;\n"
      "	gl_FragColor.b = 1.0 - col.b;\n"
      "}\n";

   vshader1 = new QOpenGLShader(QOpenGLShader::Vertex, this);
   fshader1 = new QOpenGLShader(QOpenGLShader::Fragment, this);

   if( !vshader1->compileSourceCode(vsrc1) ) {
      goto PIX_EXIT;
   }

   if( !fshader1->compileSourceCode(fsrc1) ) {
      goto PIX_EXIT;
   }

   if( !_inverse_program.addShader(vshader1) ) {
      goto PIX_EXIT;
   }

   if( !_inverse_program.addShader(fshader1) ) {
      goto PIX_EXIT;
   }

   if( !_inverse_program.link( ) ) {
     goto PIX_EXIT;
   }

   // --- DONE ---
   bRetCode = true;
PIX_EXIT:
   return bRetCode;
}

//
// create shader program from vert and frag file in qt resource
//
// notes:
// shader programs are created as separate files and included
// in the resource file(qrc) using Qt Creator
//
bool frag_pane::createSingleShader( QOpenGLShaderProgram *pShader,
                                    const QString strVertexFile,
                                    const QString strFragmentFile )
{
   bool bRetCode = false;

   // Override system locale until shaders are compiled

   setlocale(LC_NUMERIC, "C");

   // Compile vertex shader

   if( !pShader->addShaderFromSourceFile( QOpenGLShader::Vertex, strVertexFile ) ) {
      qDebug() << "Unable to compile vertex shader. Log:" << pShader->log();
      goto PIX_EXIT;
   }

   //qDebug() << "Vertex shader. Log:" << pShader->log();

   // Compile fragment shader

   if( !pShader->addShaderFromSourceFile( QOpenGLShader::Fragment, strFragmentFile ) ) {
      qDebug() << "Unable to compile fragment shader. Log:" << pShader->log();
      goto PIX_EXIT;
   }

   //qDebug() << "Fragment shader. Log:" << pShader->log();

   // Link shader pipeline

   if( !pShader->link( ) ) {
      qDebug() << "Unable to link shader program. Log:" << pShader->log();
      goto PIX_EXIT;
   }

   //qDebug() << "Linking shader program. Log:" << pShader->log();

   // Restore system locale

   setlocale(LC_ALL, "");

   // --- DONE ---
   bRetCode = true;
PIX_EXIT:
   return bRetCode;
}
/*
bool frag_pane::takeImage( cv::Mat image){

    image.copyTo(mOrigImage);

    mImgRatio = (float)image.cols/(float)image.rows;
    makeCurrent();

    mSceneChanged = true;

    updateScene();

    //PIX_EXIT:

    return true;

}
*/
bool frag_pane::showImage( cv::Mat image )
{
    image.copyTo(mOrigImage);

    mImgRatio = (float)image.cols/(float)image.rows;

    if( mOrigImage.channels() == 3)
        mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
                              mOrigImage.cols, mOrigImage.rows,
                              mOrigImage.step, QImage::Format_RGB888).rgbSwapped();
    else if( mOrigImage.channels() == 1)
        mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
                              mOrigImage.cols, mOrigImage.rows,
                              mOrigImage.step, QImage::Format_Indexed8);
    else
        return false;

    mRenderQtImg = QGLWidget::convertToGLFormat(mRenderQtImg);
    makeCurrent();

    mSceneChanged = true;

    updateScene();

    //PIX_EXIT:

    return true;
}

void frag_pane::updateScene()
{
    if( mSceneChanged && this->isVisible() )
        updateGL();
}

void frag_pane::renderImage(){



    makeCurrent();

    glClear(GL_COLOR_BUFFER_BIT);

    if (!mRenderQtImg.isNull())
    {
        glLoadIdentity();

        QImage image; // the image rendered

        glPushMatrix();
        {
            int imW = mRenderQtImg.width();
            int imH = mRenderQtImg.height();

            // The image is to be resized to fit the widget?
            if( imW != this->size().width() &&
                    imH != this->size().height() )
            {

                image = mRenderQtImg.scaled( //this->size(),
                                             QSize(mOutW,mOutH),
                                             Qt::IgnoreAspectRatio,
                                             Qt::SmoothTransformation
                                             );

                //qDebug( QString( "Image size: (%1x%2)").arg(imW).arg(imH).toAscii() );
            }
            else
                image = mRenderQtImg;

            // ---> Centering image in draw area

            glRasterPos2i( mPosX, mPosY );
            // <--- Centering image in draw area

            imW = image.width();
            imH = image.height();

            glDrawPixels( imW, imH, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
          //  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }
        glPopMatrix();

        // end
        glFlush();
    }

}

bool frag_pane::loadPic( cv::Mat pic )
{
   bool bRetCode = false;
   QImage img;
   QRgb value;
   int k, iRow, i, j;
   int nWidth, nHeight;

#ifdef _DEBUG
   QImage img2;
#endif // _DEBUG

   GLubyte *image = NULL;

       pic.copyTo(mOrigImage);

    mImgRatio = (float)pic.cols/(float)pic.rows;

    if( mOrigImage.channels() == 3)
        img = QImage((const unsigned char*)(mOrigImage.data),
                              mOrigImage.cols, mOrigImage.rows,
                              mOrigImage.step, QImage::Format_RGB888).rgbSwapped();
    else if( mOrigImage.channels() == 1)
        img = QImage((const unsigned char*)(mOrigImage.data),
                              mOrigImage.cols, mOrigImage.rows,
                              mOrigImage.step, QImage::Format_Indexed8);
    else
        return false;


    nWidth = img.width( );
    nHeight = img.height( );

    if( nWidth != this->size().width() &&
            nHeight != this->size().height() )
    {

        img = img.scaled( //this->size(),
                                     QSize(mOutW,mOutH),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation
                                     );

        //qDebug( QString( "Image size: (%1x%2)").arg(imW).arg(imH).toAscii() );
    }


    nWidth = img.width( );
    nHeight = img.height( );



   glRasterPos2i( mPosX, mPosY );

   image = new GLubyte[nHeight*nWidth*4];
   if( !image ) {
      qDebug() << "ERROR! Memory Allocation Failed.";
      goto PIX_EXIT;
   }

   // to display the texture directory onto pane, set the buffer upside down
   // to copy on FBO, use the image orientation as it is

   for( i = 0; i < nHeight; i++ ) {
      iRow = nHeight - i - 1;
      //iRow = i;
      for( j = 0 ; j < nWidth ; j++ ) {
         k = i * nWidth + j;
         value = img.pixel( j, iRow );
         image[k*4] = qRed( value );
         image[k*4+1] = qGreen( value );
         image[k*4+2] = qBlue( value );
         image[k*4+3] = 255;
      }
   }

   // create(or re-create) the FBO to set the loaded image

   if( _in_fbo ) {
      delete _in_fbo;
      _in_fbo = NULL;
   }

   makeCurrent( );
   _in_fbo = new QGLFramebufferObject( nWidth, nHeight, GL_TEXTURE_2D );
   if( !_in_fbo ) {
      goto PIX_EXIT;
   }

   // set the buffer into texture

   glEnable( GL_TEXTURE_2D );

   _in_fbo->bind( );

   glEnable( GL_TEXTURE_2D );
   glViewport( 0, 0, nWidth, nHeight );

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

    glOrtho( 0.0, nWidth, 0.0, nHeight, -1.0, 1.0 );

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity( );

   glBindTexture( GL_TEXTURE_2D, _in_fbo->texture( ) );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

   glTexImage2D( GL_TEXTURE_2D,
      0,
      GL_RGBA,
      nWidth,
      nHeight,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      image );

   _in_fbo->release();

   // --- DONE ---
   bRetCode = true;
PIX_EXIT:
   if( image ) {
      delete[] image;
      image = NULL;
   }
   return bRetCode;
}


