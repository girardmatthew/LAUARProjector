#ifndef LAUSURFACEMANAGER_H
#define LAUSURFACEMANAGER_H

#include <QDebug>
#include <QObject>
#include <QOffscreenSurface>

class LAUSurfaceManager : public QObject
{
    Q_OBJECT

public:
    LAUSurfaceManager(QObject *parent = nullptr);
    static LAUSurfaceManager *getInstance();

    ~LAUSurfaceManager();

public slots:
    void onRequestSurface(QObject *obj);

private:
    static LAUSurfaceManager *instance;

signals:
    void emitSurface(QOffscreenSurface *, QObject *);
};

LAUSurfaceManager *dLau();

#endif // LAUSURFACEMANAGER_H
