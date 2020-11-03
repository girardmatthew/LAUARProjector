CONFIG  -= basler
CONFIG  += kinect
CONFIG  -= realsense
CONFIG  -= visage
CONFIG  += opencv

QT      += core gui widgets network opengl

TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += lauvideorecordingwidget.h \
           BodyTrackingHelpers.h \
           laufacedetectorglfilter.h \
           laufacialfeaturetemplatewidget.h \
           lauprojectorglwidget.h \
           lauvideoplayerlabel.h \
           lauabstractglfilter.h \
           lausurfacemanager.h \
           lauabstractcamera.h \
           lauvideoglwidget.h \
           laumemoryobject.h \
           lauvideowidget.h \
           laulookuptable.h

SOURCES += lauvideorecordingwidget.cpp \
           laufacedetectorglfilter.cpp \
           laufacialfeaturetemplatewidget.cpp \
           lauprojectorglwidget.cpp \
           lauvideoplayerlabel.cpp \
           lauabstractglfilter.cpp \
           lausurfacemanager.cpp \
           lauvideoglwidget.cpp \
           laumemoryobject.cpp \
           lauvideowidget.cpp \
           laulookuptable.cpp \
           main.cpp

RESOURCES += $$PWD/Images/images.qrc \
             $$PWD/Shaders/shaders.qrc

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

basler {
    DEFINES += USEBASLER
    HEADERS += laubaslerglfilter.h \
               laubaslercamera.h
    SOURCES += laubaslerglfilter.cpp \
               laubaslercamera.cpp
}

realsense {
    DEFINES += USEREALSENSE
    HEADERS += laurealsenseglfilter.h \
               laurealsensecamera.h
    SOURCES += laurealsenseglfilter.cpp \
               laurealsensecamera.cpp
}

kinect {
    DEFINES += USEAZKINECT
    HEADERS += laukinectglfilter.h \
               laukinectcamera.h
    SOURCES += laukinectglfilter.cpp \
               laukinectcamera.cpp
}

unix:macx {
    CONFIG         += sdk_no_version_check
    QMAKE_CXXFLAGS += -msse2 -msse3 -mssse3 -msse4.1
    INCLUDEPATH    += /usr/local/include /usr/local/include/eigen3
    DEPENDPATH     += /usr/local/include /usr/local/include/eigen3
    LIBS           += /usr/local/lib/libtiff.5.dylib

    realsense {
        DEFINES       += REALSENSE
        INCLUDEPATH   += /usr/local/include/librealsense2
        DEPENDPATH    += /usr/local/include/librealsense2
        LIBS          += /usr/local/lib/librealsense2.dylib
    }

    basler{
        INCLUDEPATH += /Library/Frameworks/pylon.framework/Versions/A/Headers \
                       /Library/Frameworks/pylon.framework/Versions/A/Headers/GenICam
        DEPENDPATH  += /Library/Frameworks/pylon.framework/Versions/A/Headers \
                       /Library/Frameworks/pylon.framework/Versions/A/Headers/GenICam
        LIBS        += /Library/Frameworks/pylon.framework/Versions/A/pylon
    }
}

unix:!macx {
    QMAKE_CXXFLAGS += -msse2 -msse3 -mssse3 -msse4.1
    INCLUDEPATH    += /usr/include/eigen3
    DEPENDPATH     += /usr/include/eigen3
    LIBS           += -ltiff
    CONFIG         += c++11

    basler{
        INCLUDEPATH += /opt/pylon5/include \
                       /opt/pylon5/include/GenApi
        DEPENDPATH  += /opt/pylon5/include \
                       /opt/pylon5/include/GenApi
        LIBS        += -L/opt/pylon5/lib64 -lbxapi -lFirmwareUpdate_gcc_v3_1_Basler_pylon -lGCBase_gcc_v3_1_Basler_pylon -lGenApi_gcc_v3_1_Basler_pylon -lgxapi -llog4cpp_gcc_v3_1_Basler_pylon -lLog_gcc_v3_1_Basler_pylon -lMathParser_gcc_v3_1_Basler_pylon -lNodeMapData_gcc_v3_1_Basler_pylon -lpylonbase -lpylonc -lpylon_TL_bcon -lpylon_TL_camemu -lpylon_TL_gige -lpylon_TL_usb -lpylonutility -luxapi -lXmlParser_gcc_v3_1_Basler_pylon
    }

    realsense {
        DEFINES       += REALSENSE
        INCLUDEPATH   += /usr/include/librealsense2
        DEPENDPATH    += /usr/include/librealsense2
        LIBS          += -L/usr/lib/x86_64-linux-gnu -lrealsense2
    }
}

win32 {
    INCLUDEPATH += $$quote(F:/usr/include)
    DEPENDPATH  += $$quote(F:/usr/include)
    LIBS        += -L$$quote(F:/usr/lib) -llibtiff_i -lopengl32

    CONFIG(debug, debug|release):   DLLDESTDIR = $$OUT_PWD\\debug
    CONFIG(release, debug|release): DLLDESTDIR = $$OUT_PWD\\release

    basler{
        INCLUDEPATH += $$quote(C:/Program Files/Basler/pylon 5/Development/include)
        DEPENDPATH  += $$quote(C:/Program Files/Basler/pylon 5/Development/include)
        LIBS        += -L$$quote(C:/Program Files/Basler/pylon 5/Development/lib/x64/) -lPylonBase_v5_1
    }

    realsense {
        DEFINES     += REALSENSE
        INCLUDEPATH += $$quote(C:/Program Files (x86)/Intel RealSense SDK 2.0/include)
        DEPENDPATH  += $$quote(C:/Program Files (x86)/Intel RealSense SDK 2.0/include)
        LIBS        += $$quote(C:/Program Files (x86)/Intel RealSense SDK 2.0/lib/x64/realsense2.lib)
    }

    kinect {
        DEFINES     += AZUREKINECT
        #Point to path for Azure Kinect General SDK
        INCLUDEPATH += $$quote(C:/Program Files/Azure Kinect SDK v1.3.0/sdk/include)
        DEPENDPATH  += $$quote(C:/Program Files/Azure Kinect SDK v1.3.0/sdk/include)
        LIBS        += -L$$quote(C:/Program Files/Azure Kinect SDK v1.3.0/sdk/windows-desktop/amd64/release/lib) -lk4a
        #Point to path for Azure Kinect Body Tracking SDK
        INCLUDEPATH += $$quote(C:/Program Files/Azure Kinect Body Tracking SDK/sdk/include)
        DEPENDPATH  += $$quote(C:/Program Files/Azure Kinect Body Tracking SDK/sdk/include)
        LIBS        += -L$$quote(C:/Program Files/Azure Kinect Body Tracking SDK/sdk/windows-desktop/amd64/release/lib) -lk4abt
        LIBS        += -L$$quote(C:/Program Files/Azure Kinect Body Tracking SDK/sdk/windows-desktop/amd64/release/bin)
    }

    visage {
        DEFINES += USEVISAGE
        INCLUDEPATH   += $$quote(F:/usr/visageSDK/include)
        DEPENDPATH    += $$quote(F:/usr/visageSDK/include)
        LIBS += -L$$quote(F:/usr/visageSDK/lib) -llibVisageAnalyser64 -llibVisageGaze64 -llibVisageVision64
    }
}

opencv {
    DEFINES += USEOPENCV

    unix:macx {
        INCLUDEPATH   += /usr/local/include/opencv4
        DEPENDPATH    += /usr/local/include/opencv4
        LIBS          += -L/usr/local/lib -lopencv_core -lopencv_objdetect -lopencv_imgcodecs -lopencv_imgproc -lopencv_calib3d -lopencv_highgui -lopencv_ml -lopencv_face
    }

    unix:!macx {
        INCLUDEPATH   += /usr/local/include/opencv4
        DEPENDPATH    += /usr/local/include/opencv4
        LIBS          += -L/usr/local/lib -lopencv_core -lopencv_objdetect -lopencv_imgcodecs -lopencv_imgproc -lopencv_calib3d -lopencv_highgui -lopencv_ml -lopencv_face
    }

    win32 {
        INCLUDEPATH   += $$quote(F:/usr/opencv/include)
        DEPENDPATH    += $$quote(F:/usr/opencv/include)
        LIBS          += -L$$quote(F:/usr/opencv/x64/vc15/lib)
        CONFIG(release, debug|release): LIBS += -lopencv_core411 -lopencv_objdetect411 -lopencv_imgproc411 -lopencv_calib3d411 -lopencv_highgui411 -lopencv_ml411 -lopencv_face411
        CONFIG(debug, debug|release):   LIBS += -lopencv_core411d -lopencv_objdetect411d -lopencv_imgproc411d -lopencv_calib3d411d -lopencv_highgui411d -lopencv_ml411d -lopencv_face411d
    }
}
