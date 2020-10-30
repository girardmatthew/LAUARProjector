#ifndef LAUBASLERCAMERA_H
#define LAUBASLERCAMERA_H

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

#if defined(Q_OS_WIN)
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#elif defined(Q_OS_MAC)
#include <PylonIncludes.h>
#include <usb/BaslerUsbInstantCamera.h>
#elif defined(Q_OS_LINUX)
#include <Base/GCString.h>
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#endif

#include "lauabstractcamera.h"
#include "laubaslerglfilter.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUBaslerCameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAUBaslerCameraWidget(QWidget *parent = nullptr, bool hasDepth = true);

    ~LAUBaslerCameraWidget()
    {
        QSettings settings;
        settings.setValue(QString("LAUBaslerCameraWidget::exposure"), exposure);
        settings.setValue(QString("LAUBaslerCameraWidget::snrThreshold"), snrThreshold);
        settings.setValue(QString("LAUBaslerCameraWidget::mtnThreshold"), mtnThreshold);
    }

    void setExp(int val)
    {
        if (expSpinBox) {
            expSpinBox->setValue(val);
        }
    }

    void setSnr(int val)
    {
        if (snrSpinBox) {
            snrSpinBox->setValue(val);
        }
    }

    void setMtn(int val)
    {
        if (mtnSpinBox) {
            mtnSpinBox->setValue(val);
        }
    }

    int exp()
    {
        return (exposure);
    }

    int snr()
    {
        return (snrThreshold);
    }

    int mtn()
    {
        return (mtnThreshold);
    }

    void updateBuffer();

public slots:
    void onUpdateExposure(int val)
    {
        if (val != exposure) {
            exposure = val;
            if (expSpinBox) {
                expSpinBox->setValue(exposure);
            }
        }
    }

    void onUpdateSnrThreshold(int val)
    {
        if (val != snrThreshold) {
            snrThreshold = val;
            if (snrSpinBox) {
                snrSpinBox->setValue(snrThreshold);
            }
        }
    }

    void onUpdateMtnThreshold(int val)
    {
        if (val != mtnThreshold) {
            mtnThreshold = val;
            if (mtnSpinBox) {
                mtnSpinBox->setValue(mtnThreshold);
            }
        }
    }

private slots:
    void onUpdateExposurePrivate(int val)
    {
        if (val != exposure) {
            exposure = val;
            emit emitUpdateExposure(exposure);
        }
    }

    void onUpdateSnrThresholdPrivate(int val)
    {
        if (val != snrThreshold) {
            snrThreshold = val;
            emit emitUpdateSNR(snrThreshold);
        }
    }

    void onUpdateMtnThresholdPrivate(int val)
    {
        if (val != mtnThreshold) {
            mtnThreshold = val;
            emit emitUpdateMTN(mtnThreshold);
        }
    }

    void onTriggerScanPrivate()
    {
        emit emitTriggerScan();
    }

private:
    int exposure;
    int snrThreshold;
    int mtnThreshold;

    QSlider *expSlider;
    QSlider *snrSlider;
    QSlider *mtnSlider;

    QSpinBox *expSpinBox;
    QSpinBox *snrSpinBox;
    QSpinBox *mtnSpinBox;

    QPushButton *triggerScanButton;

signals:
    void emitUpdateExposure(int val);
    void emitUpdateSNR(int val);
    void emitUpdateMTN(int val);
    void emitTriggerScan();
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUBaslerCamera : public LAUAbstractCamera
{
    Q_OBJECT

public:
    explicit LAUBaslerCamera(QObject *parent = nullptr);
    ~LAUBaslerCamera();

    QWidget *widget(QWidget *parent = nullptr);

    bool reset();

protected:
    void updateBuffer();

public slots:
    void onTriggerScan()
    {
        mode = ModeScan;
    }

    void onUpdateSNR(int val)
    {
        localSNR = val;
        if (sliFilter) {
            sliFilter->onSetSNRThreshold(static_cast<unsigned int>(val));
        }
    }

    void onUpdateMTN(int val)
    {
        localMTN = val;
        if (sliFilter) {
            sliFilter->onSetMTNThreshold(static_cast<unsigned int>(val));
        }
    }

    void onUpdateExposure(int microseconds);

private:
    enum Modality { ModeScan, ModeFace } mode;

    static bool libraryInitializedFlag;

    // DECLARE POINTERS TO PRIMESENSE SENSOR OBJECTS
    unsigned int numAvailableCameras;
    Pylon::CBaslerUsbInstantCamera *camera;
    QStringList cameraList();

    int localSNR, localMTN;

    void debayer(unsigned char *otBuffer, unsigned char *inBuffer, int rows, int cols, int step);
    bool setSynchronization();
    bool disconnectFromHost();
    bool connectToHost(QString);
    bool isColor;

    LAUMemoryObject frameBuffer;
    LAUBaslerGLFilter *sliFilter;
};

#endif // LAUBASLERCAMERA_H
