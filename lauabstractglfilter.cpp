#include "lauabstractglfilter.h"
#include "locale.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUAbstractGLFilter::~LAUAbstractGLFilter()
{
    while (mutex.tryLock() == false) {
        qApp->processEvents();
    }
    mutex.unlock();
    qDebug() << QString("LAUAbstractGLFilter::~LAUAbstractGLFilter()");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::onSetSurface(QOffscreenSurface *srfc, QObject *obj)
{
    if (srfc == nullptr) {
        // WE NEED TO REQUEST AN OFFSCREEN SURFACE FROM THE SURFACE MANAGER
        connect(this, SIGNAL(emitRequestForSurface(QObject *)), dLau(), SLOT(onRequestSurface(QObject *)), Qt::QueuedConnection);
        connect(dLau(), SIGNAL(emitSurface(QOffscreenSurface *, QObject *)), this, SLOT(onSetSurface(QOffscreenSurface *, QObject *)), Qt::QueuedConnection);
        emit emitRequestForSurface(this);
    } else if (obj == this) {
        // KEEP A LOCAL COPY OF THE INCOMING SURFACE
        surface = srfc;

        // CREATE THE GLFILTER AND INITIALIZE IT
        this->setFormat(surface->format());
        this->create();
        this->onInitialize();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::onInitialize()
{
    // WE NEED TO MAKE OURSELVES THE CURRENT CONTEXT
    if (surface && makeCurrent(surface)) {
        // SET UP CONNECTIONS BETWEEN QT OPENGL CLASSES AND UNDERLYING OPENGL DRIVERS
        initializeOpenGLFunctions();

        // GET CONTEXT OPENGL-VERSION
        qDebug() << "Really used OpenGl: " << format().majorVersion() << "." << format().minorVersion();
        qDebug() << "OpenGl information: VENDOR:       " << reinterpret_cast<const char *>(glGetString(GL_VENDOR));
        qDebug() << "                    RENDERDER:    " << reinterpret_cast<const char *>(glGetString(GL_RENDERER));
        qDebug() << "                    VERSION:      " << reinterpret_cast<const char *>(glGetString(GL_VERSION));
        qDebug() << "                    GLSL VERSION: " << reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.create();
        vertexArrayObject.bind();

        // CREATE A BUFFER TO HOLD THE ROW AND COLUMN COORDINATES OF IMAGE PIXELS FOR THE TEXEL FETCHES
        quadVertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        quadVertexBuffer.create();
        quadVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
        if (quadVertexBuffer.bind()) {
            quadVertexBuffer.allocate(16 * sizeof(float));
            float *vertices = reinterpret_cast<float *>(quadVertexBuffer.map(QOpenGLBuffer::WriteOnly));
            if (vertices) {
                vertices[0]  = -1.0;
                vertices[1]  = -1.0;
                vertices[2]  = 0.0;
                vertices[3]  = 1.0;
                vertices[4]  = +1.0;
                vertices[5]  = -1.0;
                vertices[6]  = 0.0;
                vertices[7]  = 1.0;
                vertices[8]  = +1.0;
                vertices[9]  = +1.0;
                vertices[10] = 0.0;
                vertices[11] = 1.0;
                vertices[12] = -1.0;
                vertices[13] = +1.0;
                vertices[14] = 0.0;
                vertices[15] = 1.0;

                quadVertexBuffer.unmap();
            } else {
                qDebug() << QString("Unable to map quadVertexBuffer from GPU.");
            }
        }

        // CREATE AN INDEX BUFFER FOR THE INCOMING DEPTH VIDEO DRAWN AS POINTS
        quadIndexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        quadIndexBuffer.create();
        quadIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
        if (quadIndexBuffer.bind()) {
            quadIndexBuffer.allocate(6 * sizeof(unsigned int));
            unsigned int *indices = reinterpret_cast<unsigned int *>(quadIndexBuffer.map(QOpenGLBuffer::WriteOnly));
            if (indices) {
                indices[0] = 0;
                indices[1] = 1;
                indices[2] = 2;
                indices[3] = 0;
                indices[4] = 2;
                indices[5] = 3;
                quadIndexBuffer.unmap();
            } else {
                qDebug() << QString("quadIndexBuffer buffer mapped from GPU.");
            }
        }

        // SET THE ALIGNMENT OPTION FOR TRANSFERING PIXEL DATA FOR OUR TEXTURES TO THE GPU
        options.setAlignment(1);

        // CALL SUB-CLASSES INITIALIZE FUNCTION
        initialize();

        // RELEASE THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractGLFilter::flush()
{
    if (surface && makeCurrent(surface)) {
        glFlush();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUAbstractFilterController::LAUAbstractFilterController(LAUAbstractGLFilter *contxt, QSurface *srfc, QObject *parent) : QObject(parent), localContext(contxt), localSurface(false), surface(srfc), thread(nullptr)
{
    // SEE IF THE USER GAVE US A TARGET SURFACE, IF NOT, THEN CREATE AN OFFSCREEN SURFACE BY DEFAULT
    if (surface == nullptr) {
        surface = new QOffscreenSurface();
        ((QOffscreenSurface *)surface)->create();
        localSurface = true;
    }

    // NOW SEE IF WE HAVE A VALID PROCESSING CONTEXT FROM THE USER, AND THEN SPIN IT INTO ITS OWN THREAD
    if (localContext) {
        localContext->setFormat(surface->format());
        localContext->onSetSurface((QOffscreenSurface *)surface, localContext);
        if (localContext->isValid()) {
            thread = new QThread();
            localContext->moveToThread(thread);
            thread->start();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUAbstractFilterController::~LAUAbstractFilterController()
{
    if (localContext) {
        localContext->deleteLater();
    }

    if (thread) {
        thread->quit();
        while (thread->isRunning()) {
            qApp->processEvents();
        }
        delete thread;
    }

    if (localSurface && surface) {
        delete surface;
    }
}
