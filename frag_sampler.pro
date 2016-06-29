QT += core
QT += gui
QT += opengl

CONFIG += app
CONFIG += console
TARGET = frag_sampler

SOURCES += \
    frag_main.cpp \
    frag_pane.cpp \
    frag_window.cpp \
    frag_window_ut.cpp \
    glpipe.cpp

OTHER_FILES += \
    shaders/inverse.vert \
    shaders/inverse.frag \
    shaders/median3.vert \
    shaders/median3.frag \
    shaders/mosaic.frag \
    shaders/mosaic.vert \
    shaders/copy.frag \
    shaders/copy.vert

HEADERS += \
    frag_pane.h \
    frag_window.h \
    cvqtviewer.h

FORMS += \
    frag_window.ui


OPENCV_PATH = C:\opencv2410\

RESOURCES += \
    frag_sampler.qrc

QMAKE_LFLAGS += /INCREMENTAL:NO


INCLUDEPATH += C:\glew\include\

INCLUDEPATH += C:\opencv\build\include\

win32 {
    INCLUDEPATH += "C:\\opencv2410\\build\\include" \

    CONFIG(debug,debug|release) {
        LIBS += -L"C:\\opencv2410\\build\\x86\\vc12\\lib" \
            -lopencv_core2410d \
            -lopencv_highgui2410d \
            -lopencv_imgproc2410d \
            -lopencv_features2d2410d \
            -lopencv_calib3d2410d
    }

    CONFIG(release,debug|release) {
        DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT
        LIBS += -L"C:\\opencv2410\\build\\x86\\vc12\\lib" \
            -lopencv_core2410 \
            -lopencv_highgui2410 \
            -lopencv_imgproc2410 \
            -lopencv_features2d2410 \
            -lopencv_calib3d2410
    }
}


unix {
message("Using unix configuration")

OPENCV_PATH = /usr/local/opencv2/

LIBS     += -L$$LIBS_PATH \
            -lopencv_core \
            -lopencv_highgui
}

INCLUDEPATH += $$OPENCV_PATH/modules/core/include/ \ #core module
    $$OPENCV_PATH/modules/highgui/include/ #highgui modul

message("OpenCV path: $$OPENCV_PATH")
message("Includes path: $$INCLUDEPATH")
message("Libraries: $$LIBS")

