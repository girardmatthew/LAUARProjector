/*********************************************************************************
 *                                                                               *
 * Copyright (c) 2017, Dr. Daniel L. Lau                                         *
 * All rights reserved.                                                          *
 *                                                                               *
 * Redistribution and use in source and binary forms, with or without            *
 * modification, are permitted provided that the following conditions are met:   *
 * 1. Redistributions of source code must retain the above copyright             *
 *    notice, this list of conditions and the following disclaimer.              *
 * 2. Redistributions in binary form must reproduce the above copyright          *
 *    notice, this list of conditions and the following disclaimer in the        *
 *    documentation and/or other materials provided with the distribution.       *
 * 3. All advertising materials mentioning features or use of this software      *
 *    must display the following acknowledgement:                                *
 *    This product includes software developed by the <organization>.            *
 * 4. Neither the name of the <organization> nor the                             *
 *    names of its contributors may be used to endorse or promote products       *
 *    derived from this software without specific prior written permission.      *
 *                                                                               *
 * THIS SOFTWARE IS PROVIDED BY Dr. Daniel L. Lau ''AS IS'' AND ANY              *
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED     *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        *
 * DISCLAIMED. IN NO EVENT SHALL Dr. Daniel L. Lau BE LIABLE FOR ANY             *
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;  *
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND   *
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                  *
 *                                                                               *
 *********************************************************************************/

#ifndef LAUVIDEOWIDGET_H
#define LAUVIDEOWIDGET_H

#include <QTime>
#include <QDebug>
#include <QImage>
#include <QTimer>
#include <QWidget>
#include <QAction>
#include <QThread>
#include <QVBoxLayout>
#include <QMessageBox>

#include "laumemoryobject.h"
#include "lauvideoglwidget.h"
#include "lauvideoplayerlabel.h"
#include "lauprojectorglwidget.h"
#include "laufacedetectorglfilter.h"
#include "laufacialfeaturetemplatewidget.h"

#if defined(USEBASLER)
#include "laubaslercamera.h"
#elif defined(USEREALSENSE)
#include "laurealsensecamera.h"
#elif defined(USEAZKINECT)
#include "laukinectcamera.h"
#endif

class LAUVideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAUVideoWidget(QWidget *parent = nullptr);
    ~LAUVideoWidget();

    QSize size() const
    {
        return (QSize((int)camera->width(), (int)camera->height()));
    }

    int width() const
    {
        return (size().width());
    }

    int height() const
    {
        return (size().height());
    }

    QString make() const
    {
        return (camera->make());
    }

    QString model() const
    {
        return (camera->model());
    }

    QString serial() const
    {
        return (camera->serial());
    }

    virtual bool isValid() const
    {
        if (camera) {
            return (camera->isValid());
        } else {
            return (false);
        }
    }

    virtual bool isNull() const
    {
        return (!isValid());
    }

    void insertAction(QAction *action);
    void insertActions(QList<QAction *> actions);

    LAUMemoryObject grabScreenShot()
    {
        if (glWidget) {
            return (glWidget->grabScreenShot());
        }
        return (LAUMemoryObject());
    }

public slots:
    void onSetFaceTemplate();
    void onContextMenuTriggered();
    void onCameraError(QString string)
    {
        qDebug() << string;
    }

protected:
    void keyPressEvent(QKeyEvent *event)
    {
        qDebug() << "keyPressEvent(QKeyEvent *event)" << event->key();
    }

    void hideEvent(QHideEvent *)
    {
        // TELL CAMERA TO START GRABBING
        emit emitEnableCamera(false);

        // HIDE THE PROJECTOR GLWIDGET
        if (projector) {
            projector->hide();
        }
    }

    void showEvent(QShowEvent *);

    int counter;
    QTime time;
    QWidget *cameraWidget;
    QThread *cameraThread;

    QWidget *projectorWidget;
    LAUAbstractCamera *camera;
    LAUVideoGLWidget *glWidget;
    LAUFaceDetectorGLFilter *faceGLFilter;
    LAUProjectorGLWidget *projector;
    LAUAbstractFilterController *faceDetectorController;

signals:
    void emitEnableCamera(bool);
};

#endif // LAUVIDEOWIDGET_H
