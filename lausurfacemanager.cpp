#include "lausurfacemanager.h"

LAUSurfaceManager *LAUSurfaceManager::instance = nullptr;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUSurfaceManager *dLau()
{
    return (LAUSurfaceManager::getInstance());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUSurfaceManager::LAUSurfaceManager(QObject *parent) : QObject(parent)
{
    instance = this;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUSurfaceManager::~LAUSurfaceManager()
{
    qDebug() << "LAUSurfaceManager::~LAUSurfaceManager()";
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUSurfaceManager *LAUSurfaceManager::getInstance()
{
    if (instance == nullptr) {
        instance = new LAUSurfaceManager();
    }
    return instance;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUSurfaceManager::onRequestSurface(QObject *obj)
{
    // GRAB THE FIRST UNUSED SURFACE
    QOffscreenSurface *surface = new QOffscreenSurface();
    reinterpret_cast<QOffscreenSurface *>(surface)->create();

    // CONNECT THE DESTROYED SIGNAL SO WE CAN DELETE THE SURFACE
    connect(obj, SIGNAL(destroyed()), surface, SLOT(deleteLater()));

    // MOVE THE NEW SURFACE TO THE SAME THREAD AS THE REQUESTING OBJECT
    surface->moveToThread(obj->thread());

    // SEND THE CREATED SURFACE BACK TO THE CALLER
    emit emitSurface(surface, obj);
}
