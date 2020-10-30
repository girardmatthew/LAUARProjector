#ifndef LAUABSTRACTCAMERA_H
#define LAUABSTRACTCAMERA_H

#include <QTime>
#include <QList>
#include <QTimer>
#include <QtCore>
#include <QDebug>
#include <QImage>
#include <QDebug>
#include <QLabel>
#include <QMutex>
#include <QString>
#include <QObject>
#include <QThread>
#include <QSlider>
#include <QWidget>
#include <QSpinBox>
#include <QSettings>
#include <QMessageBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QHostAddress>
#include <QDoubleSpinBox>

#include "laulookuptable.h"
#include "lauabstractglfilter.h"

#define ABSTRACTROWS 480
#define ABSTRACTCOLS 640

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUAbstractCamera : public QObject
{
    Q_OBJECT

public:
    explicit LAUAbstractCamera(QObject *parent = nullptr) : QObject(parent), numRows(0), numCols(0), numFrms(0), numBits(0), isRotated(false), isRunning(false), isConnected(false), shareContext(nullptr) { }

    ~LAUAbstractCamera()
    {
        onEnableVideo(false);
        while (mutex.tryLock() == false) {
            qApp->processEvents();
        }
        mutex.unlock();
        qDebug() << "LAUAbstractCamera::~LAUAbstractCamera()";
    }

    virtual bool reset()
    {
        return (false);
    }

    bool portrait() const
    {
        return (isRotated);
    }

    void enablePortraitMode(bool state = true)
    {
        isRotated = state;
    }

    void disablePortraitMode(bool state = true)
    {
        isRotated = !state;
    }

    void setLookUpTable(LAULookUpTable lut)
    {
        lookUpTable = lut;
    }

    void setShareContext(QOpenGLContext *context)
    {
        shareContext = context;
    }

    virtual bool isValid() const
    {
        return (isConnected);
    }

    unsigned short maxIntensityValue() const
    {
        return (static_cast<unsigned short>((0x01 << numBits) - 1));
    }

    QString error() const
    {
        return (errorString);
    }

    QString make() const
    {
        return (makeString);
    }

    QString model() const
    {
        return (modelString);
    }

    QString serial() const
    {
        return (serialString);
    }

    unsigned int width() const
    {
        if (isRotated) {
            return (numRows);
        } else {
            return (numCols);
        }
    }

    unsigned int height() const
    {
        if (isRotated) {
            return (numCols);
        } else {
            return (numRows);
        }
    }

    unsigned int frames() const
    {
        return (numFrms);
    }

    virtual QWidget *widget(QWidget *parent = nullptr)
    {
        return (new QWidget(parent));
    }

    void timerEvent(QTimerEvent *)
    {
        if (mutex.tryLock()) {
            updateBuffer();
            mutex.unlock();
        }
    }

public slots:
    void onEnableVideo(bool state = true)
    {
        if (isRunning != state) {
            isRunning = state;
            if (isRunning) {
                timerId = startTimer(5, Qt::PreciseTimer);
            } else {
                killTimer(timerId);
            }
        }
    }

protected:
    virtual void updateBuffer() = 0;

    unsigned int numRows;
    unsigned int numCols;
    unsigned int numFrms;
    unsigned int numBits;

    bool isRotated;
    bool isRunning;
    bool isConnected;

    QString makeString;
    QString errorString;
    QString modelString;
    QString serialString;

    QMutex mutex;
    LAULookUpTable lookUpTable;
    QOpenGLContext *shareContext;

private:
    int timerId;

signals:
    void emitError(QString);
    void emitScanFBO(QOpenGLFramebufferObject *);
    void emitFaceFBO(QOpenGLFramebufferObject *);
    void emitVideFBO(QOpenGLFramebufferObject *);
    void emitDeptFBO(QOpenGLFramebufferObject *);
};

#endif // LAUABSTRACTCAMERA_H
