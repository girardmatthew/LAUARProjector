#ifndef LAUFACEDETECTORGLFILTER_H
#define LAUFACEDETECTORGLFILTER_H

#include <QtCore>
#include <QObject>
#include <QSettings>
#include <QFileDialog>
#include <QApplication>

#ifdef USEOPENCV
#include "opencv2/face.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#endif

#ifdef USEVISAGE
#include "visageVision.h"
#include "VisageTracker.h"
#endif

#include "laumemoryobject.h"
#include "lauabstractglfilter.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUFaceDetectorGLFilter : public LAUAbstractGLFilter
{
    Q_OBJECT

public:
    explicit LAUFaceDetectorGLFilter(unsigned int cols, unsigned int rows, QObject *parent = nullptr);
    ~LAUFaceDetectorGLFilter();

    bool hasFaceDetector()
    {
#ifdef USEOPENCV
        if (facemark) {
            return (true);
        }
#endif
        return (false);
    }

    void initialize();
    QOpenGLFramebufferObject *updateFBO(QOpenGLFramebufferObject *fbo);

    static QString faceDetectorString;
    static QString faceMarkString;
    static QList<QPointF> detectFacialFeatures(LAUMemoryObject object);

private:
    //LAUMemoryObject objectA;
    LAUMemoryObject faceVertexObject;

    QOpenGLFramebufferObject *frameBufferObject;
    QOpenGLShaderProgram program;

#ifdef USEVISAGE
    VsImage *inputImage;
    VisageSDK::VisageTracker *visageTracker;
    VisageSDK::FaceData faceData[16];
#endif

#ifdef USEOPENCV
    cv::Mat grayFrame;
    cv::Ptr<cv::CascadeClassifier> faceDetector;
    cv::Ptr<cv::face::Facemark> facemark;
#endif

signals:
    void emitFaceVertices(QList<QPointF> points);
};

#endif // LAUFACEDETECTORGLFILTER_H
