#ifndef LAUKINECTCAMERA_H
#define LAUKINECTCAMERA_H

#include <QTime>
#include <QList>
#include <QTimer>
#include <QtCore>
#include <QDebug>
#include <QImage>
#include <QDebug>
#include <QLabel>
#include <QString>
#include <QObject>
#include <QThread>
#include <QSlider>
#include <QWidget>
#include <QSpinBox>
#include <QSettings>
#include <QPushButton>
#include <QMessageBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QHostAddress>
#include <QDoubleSpinBox>

#define NOMINMAX

#ifdef AZUREKINECT
#include "k4a/k4a.h"
#include "k4abt.h"
#include "BodyTrackingHelpers.h"
#else
#include "Kinect.h"
#endif

#include "lauabstractcamera.h"
#include "laukinectglfilter.h"

//Define body tracker as 1 to turn on body tracking and 0 to turn off body tracking
#define body_tracker 1

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUKinectCamera : public LAUAbstractCamera
{
    Q_OBJECT

public:
    explicit LAUKinectCamera(QObject *parent = nullptr);
    ~LAUKinectCamera();

    bool reset()
    {
        return (false);
    }

protected:
    void updateBuffer();

public slots:

private:
    unsigned int rgbCols, rgbRows;

#ifdef AZUREKINECT
    k4a_device_t ptrKinectSensor;
    k4abt_tracker_t tracker;
#else
    // DECLARE POINTERS TO KINECT SENSOR OBJECTS
    IKinectSensor *ptrKinectSensor;
    IDepthFrameReader *ptrDepthFrameReader;
    ICoordinateMapper *ptrCoordinateMapper;
    IColorFrameReader *ptrColorFrameReader;
    IMultiSourceFrameReader *ptrMultiSourceFrameReader;
    IInfraredFrameReader *ptrInfraredFrameReader;
#endif

    LAUMemoryObject scanBuffer;
    LAUKinectGLFilter *glFilter;
};

#endif // LAUKINECTCAMERA_H
