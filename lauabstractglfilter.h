#ifndef LAUABSTRACTSLIGLFILTER_H
#define LAUABSTRACTSLIGLFILTER_H

#include <QtCore>
#include <QObject>
#include <QThread>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLPixelTransferOptions>

#include "lausurfacemanager.h"

#ifndef PI
#define PI 3.141592653589793
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUAbstractGLFilter : public QOpenGLContext, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit LAUAbstractGLFilter(unsigned int cols, unsigned int rows, QObject *parent) : QOpenGLContext(parent), numCols(cols), numRows(rows), isRotated(false), isSurfaceLocal(false), isWaitingForSurface(false), surface(nullptr) { ; }
    ~LAUAbstractGLFilter();

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

    bool isValid() const
    {
        return (wasInitialized());
    }

    bool wasInitialized() const
    {
        return (vertexArrayObject.isCreated());
    }

    unsigned int width() const
    {
        return (numCols);
    }

    unsigned int height() const
    {
        return (numRows);
    }

    virtual void initialize() = 0;
    virtual QOpenGLFramebufferObject *updateFBO(QOpenGLFramebufferObject *fbo)
    {
        return (fbo);
    }
    void flush();

public slots:
    void onInitialize();
    void onUpdateFBO(QOpenGLFramebufferObject *fbo)
    {
        if (mutex.tryLock()) {
            fbo = updateFBO(fbo);
            flush();
            emit emitFrameBufferObject(fbo);
            qApp->processEvents();
            mutex.unlock();
        }
    }
    void onSetSurface(QOffscreenSurface *srfc = nullptr, QObject *obj = nullptr);

protected:
    unsigned int numCols, numRows;

    bool isBusy;
    bool isRotated;
    bool isSurfaceLocal;
    bool isWaitingForSurface;

    QMutex mutex;
    QSurface *surface;
    QOpenGLPixelTransferOptions options;
    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLBuffer quadVertexBuffer, quadIndexBuffer;

signals:
    void emitRequestForSurface(QObject *);
    void emitFrameBufferObject(QOpenGLFramebufferObject *);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUAbstractFilterController : public QObject
{
    Q_OBJECT

public:
    explicit LAUAbstractFilterController(LAUAbstractGLFilter *contxt, QSurface *srfc = nullptr, QObject *parent = nullptr);
    explicit LAUAbstractFilterController(LAUAbstractGLFilter *contxt, QOpenGLWidget *wdgt, QObject *parent = nullptr);
    ~LAUAbstractFilterController();

    LAUAbstractGLFilter *glFilter() const
    {
        return (localContext);
    }

protected:
    LAUAbstractGLFilter *localContext;
    bool localSurface;
    QSurface *surface;
    QThread *thread;
};

#endif // LAUABSTRACTSLIGLFILTER_H
