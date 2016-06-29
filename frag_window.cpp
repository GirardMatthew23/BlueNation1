#include "frag_pane.h"
#include "frag_window.h"
#include "ui_frag_window.h"
//#include "cvqtviewer.h"

frag_window::frag_window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frag_window)
{
   QIcon icon;

   ui->setupUi(this);

   _frag_pane = new frag_pane;  
   _frag_pane->setParent( this );  

   setCentralWidget( _frag_pane );
   
   restoreWindowPosition( );

   icon = QIcon( ":/images/Moon.png" );
   setWindowIcon( icon );

   return;
}

frag_window::~frag_window()
{
   delete ui;
}


void frag_window::closeEvent( QCloseEvent * /*eve*/ )
{
   storeWindowPosition( );

   return;
}

// *** file ***
// *** file ***
void frag_window::on_actionTake_Pic_triggered()
{
   picTaken = true;
   videoStream = false;
   if( !selectSingleImage( picTaken) ) {
      goto PIX_EXIT;
   }

PIX_EXIT:
   return;
}


void frag_window::on_actionStart_Camera_triggered()
{
    videoStream = true;
    if( !mCapture.isOpened() )
        if( !mCapture.open( 0 ) )
            return;

    startTimer(0);
}

void frag_window::on_action_file_open_image_triggered()
{
   picTaken = false;
   videoStream = false;
   if( !selectSingleImage( picTaken ) ) {
      goto PIX_EXIT;
   }

PIX_EXIT:
   return;
}

void frag_window::on_action_file_save_image_triggered()
{
   frag_pane *pPane = getPane( );  

   if( !pPane->validImageProc( ) ) {
      goto PIX_EXIT;
   }
   
   if( !saveSingleImage( ) ) {
      goto PIX_EXIT;
   }

PIX_EXIT:
   return;
}

void frag_window::on_action_file_read_supported_format_triggered()
{
    listSupportedFormats( true );

    return;
}

void frag_window::on_action_file_write_supported_format_triggered()
{
    listSupportedFormats( false );

    return;
}

void frag_window::on_action_file_exit_triggered()
{
    close( );
}

// *** filter ***

void frag_window::on_action_filter_original_triggered()
{
   frag_pane *pPane = getPane( );  

   if( !pPane->validImageProc( ) ) {
      goto PIX_EXIT;
   }
   
   pPane->setImageProc( frag_pane::ImageProcOriginal );
   
   if( !pPane->setTexture( ) ) {
      pPane->resetImageProc( );
      pPane->refreshImage( );
      goto PIX_EXIT;
   }
   
   pPane->refreshImage( );

PIX_EXIT:
   return;
}

void frag_window::on_action_filter_inverse_triggered()
{
   frag_pane *pPane = getPane( );

   if( !pPane->validImageProc( ) ) {
       goto PIX_EXIT;
    }

   pPane->setImageProc( frag_pane::ImageProcInverse );
       
   if( !pPane->setTexture( ) ) {
      pPane->resetImageProc( );
      pPane->refreshImage( );
      goto PIX_EXIT;
   }
   
   pPane->refreshImage( );

 PIX_EXIT:
    return;
}

void frag_window::on_action_filter_mosaic_triggered()
{

    mosaicShader = true;
    frag_pane *pPane = getPane( );

    if( !pPane->validImageProc( ) ) {
        goto PIX_EXIT;
     }

    pPane->setImageProc( frag_pane::ImageProcMosaic );

    if( !pPane->setTexture( ) ) {
       pPane->resetImageProc( );
       pPane->refreshImage( );
       goto PIX_EXIT;
    }

    pPane->refreshImage( );


  PIX_EXIT:
    return;
}

void frag_window::on_action_filter_median_3X3_triggered()
{
   frag_pane *pPane = getPane( );

   if( !pPane->validImageProc( ) ) {
       goto PIX_EXIT;
    }

   pPane->setImageProc( frag_pane::ImageProcMedian3X3 );
       
   if( !pPane->setTexture( ) ) {
      pPane->resetImageProc( );
      pPane->refreshImage( );
      goto PIX_EXIT;
   }
   
   pPane->refreshImage( );

 PIX_EXIT:
   return;
}

void frag_window::on_actionColor_Reduce_triggered(){
    colorReduce = !colorReduce;
}

void frag_window::on_actionLResample_triggered(){

/*    bool ok;
    int nIMh = QInputDialog::getInt(this, tr("QInputDialog::getInteger()"),
                                 tr("What is new height in pixels?"), 25, 0, 4000, 1, &ok);
    int nIMw = QInputDialog::getInt(this, tr("QInputDialog::getInteger()"),
                                 tr("What is new width in pixels?"), 25, 0, 4000, 1, &ok);


    QString h = QString::number(nIMh);
    QString w = QString::number(nIMw);

    if (ok){
        QMessageBox msgBox;
        QString msg;
                msg += "Height: ";
                msg += h;
                msg += "    Width: ";
                msg += w;
        msgBox.setText(msg);
        msgBox.exec();}
*/

    lresample = true;
    frag_pane *pPane = getPane( );

    if( !pPane->validImageProc( ) ) {
        goto PIX_EXIT;
     }

    pPane->setImageProc( frag_pane::ImageProcLResample );

    if( !pPane->setTexture( ) ) {
       pPane->resetImageProc( );
       pPane->refreshImage( );
       goto PIX_EXIT;
    }

    pPane->refreshImage( );


  PIX_EXIT:
    return;

}

void frag_window::lresample_loop(){

    frag_pane *pPane = getPane( );

    if( !pPane->validImageProc( ) ) {
        goto PIX_EXIT;
     }

    pPane->setImageProc( frag_pane::ImageProcLResample );

    if( !pPane->setTexture( ) ) {
       pPane->resetImageProc( );
       pPane->refreshImage( );
       goto PIX_EXIT;
    }

    pPane->refreshImage( );


  PIX_EXIT:
    return;
}



void frag_window::timerEvent(QTimerEvent *event)
{
    int div=64;
    cv::Mat pic;
    mCapture >> pic;
    // Do what you want with the image :-)
    if (colorReduce)
{
    int nl= pic.rows;
    int nc= pic.cols * pic.channels();

    for (int j=0; j<nl; j++) {
        uchar* data= pic.ptr<uchar>(j);
        for (int i=0; i<nc; i++){

            data[i]= data[i]/div*div + div/2;
        }
    }
}
    // Show the image
    //ui->openCVviewer->takeImage( pic );
    cv::imshow("Camera image", pic);  // display live camera stream on another window

    if(mosaicShader)
        on_action_filter_mosaic_triggered();
    if(lresample)
        on_actionLResample_triggered();

    if(videoStream)
   if( !selectSingleImage( videoStream) ) {
      goto PIX_EXIT;
   }

PIX_EXIT:
   return;
}
