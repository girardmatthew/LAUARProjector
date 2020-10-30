#ifndef LAUREALSENSECAMERA_H
#define LAUREALSENSECAMERA_H

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

#include <librealsense2/rs.h>
#include <librealsense2/rsutil.h>
#include <librealsense2/h/rs_pipeline.h>
#include <librealsense2/h/rs_option.h>
#include <librealsense2/h/rs_frame.h>

#include "lauabstractcamera.h"
#include "laurealsenseglfilter.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURealSenseCamera : public LAUAbstractCamera
{
    Q_OBJECT

public:
    explicit LAURealSenseCamera(QObject *parent = nullptr);
    ~LAURealSenseCamera();

    bool reset()
    {
        return (false);
    }

protected:
    void updateBuffer();

public slots:

private:
    typedef struct {
        int index;
        int uniqueID;
        int frameRate;
        rs2_stream stream;
        rs2_format format;
        const rs2_stream_profile *profile;
    } StreamPacket;

    typedef struct {
        QString sensorString;
        QStringList supportOptions;
        QList<StreamPacket> streams;
    } SensorPacket;

    QList<SensorPacket> sensors;

    // DECLARE POINTERS TO REALSENSE SENSOR OBJECTS
    rs2_context *context;
    rs2_device_list *cameraList;
    rs2_device *camera;
    rs2_pipeline *pipeline;
    rs2_config *configure;
    rs2_pipeline_profile *profile;
    rs2_stream colorStream;
    rs2_intrinsics intrinsicsClr;
    rs2_intrinsics intrinsicsDpt;

    LAUMemoryObject scanBuffer;
    LAURealSenseGLFilter *glFilter;

    // METHOD FOR SEARCHING ALL AVAILABLE STREAMS FOR A GIVEN SENSOR
    StreamPacket findStream(SensorPacket sensor, rs2_stream stream, rs2_format format, int wdth, int hght, int fps);
};

#endif // LAUREALSENSECAMERA_H
