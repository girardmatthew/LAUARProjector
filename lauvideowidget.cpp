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
#include "lauvideowidget.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUVideoWidget::LAUVideoWidget(QWidget *parent) : QWidget(parent), counter(0), cameraWidget(nullptr), cameraThread(nullptr), projectorWidget(nullptr), camera(nullptr), glWidget(nullptr), faceGLFilter(nullptr),  projector(nullptr), faceDetectorController(nullptr)
{
    this->setLayout(new QVBoxLayout());
    this->setWindowTitle(QString("Video Recorder"));
    this->setFocusPolicy(Qt::StrongFocus);
    this->layout()->setContentsMargins(0, 0, 0, 0);
    this->layout()->setSpacing(0);

#if defined(USEBASLER)
    camera = new LAUBaslerCamera();
#elif defined(USEREALSENSE)
    camera = new LAURealSenseCamera();
#elif defined(USEAZKINECT)
    camera = new LAUKinectCamera();
#endif
    if (camera && camera->isValid()) {
#if defined(USEREALSENSE) || defined(USEAZKINECT)
        camera->enablePortraitMode(false);
        camera->setLookUpTable(LAULookUpTable((QString())));
#endif
        // CREATE A GLWIDGET TO DISPLAY RGB VIDEO ON THE USER SCREEN
        glWidget = new LAUVideoGLWidget(camera->width(), camera->height());
        glWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        glWidget->setMaximumIntensityValue(camera->maxIntensityValue());
        this->layout()->addWidget(glWidget);

        // MAKE OUR SIGNAL AND SLOT CONNECTIONS
        connect(camera, SIGNAL(emitError(QString)), this, SLOT(onCameraError(QString)), Qt::QueuedConnection);
        connect(this, SIGNAL(emitEnableCamera(bool)), camera, SLOT(onEnableVideo(bool)), Qt::QueuedConnection);

        // CREATE A CAMERA WIDGET
        cameraWidget = camera->widget(this);

        // CREATE A MENU ACTION FOR DISPLAYING THE CAMERA WIDGET
        QAction *action = new QAction(QString("Adjust camera settings..."), nullptr);
        action->setCheckable(false);
        connect(action, SIGNAL(triggered()), this, SLOT(onContextMenuTriggered()));
        glWidget->menu()->addAction(action);

        action = new QAction(QString("Set Face Template..."), nullptr);
        action->setCheckable(false);
        connect(action, SIGNAL(triggered()), this, SLOT(onSetFaceTemplate()));
        glWidget->menu()->addAction(action);

        // CREATE AN OPENGL WIDGET TO CONTROL THE SANDBOX PROJECTOR
        projector = new LAUProjectorGLWidget(camera->width(), camera->height(), 1);
        projector->show();

        // MOVE THE CAMERA TO ITS OWN THREAD
        cameraThread = new QThread();
        camera->moveToThread(cameraThread);
        cameraThread->start();

        // WAIT HERE UNTIL WE KNOW THREAD IS RUNNING
        if (cameraThread->isRunning() == false) {
            qApp->processEvents();
        }
    } else if (camera) {
        // CREATE A NO VIDEO OPENGL WIDGET FOR THE USER DISPLAY
        glWidget = new LAUVideoGLWidget(320, 240);
        glWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->layout()->addWidget(glWidget);

        // CREATE A PROJECTOR WIDGET ON THE SECONDARY SCREEN, IF IT EXISTS
        if (QGuiApplication::screens().count() > 1) {
            projector = new LAUProjectorGLWidget(320, 240, 1);
            projector->setGeometry(QGuiApplication::screens().last()->geometry());
            projector->show();
        }

        // TELL THE USER WHAT WENT WRONG WITH THE CAMERA
        QMessageBox::warning(this, QString("AR Sandbox"), camera->error());
    } else {
        // TELL THE USER WHAT WENT WRONG
        QMessageBox::warning(this, QString("AR Sandbox"), QString("nullptr camera object."));
        this->layout()->addWidget(new LAUVideoGLWidget());
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUVideoWidget::~LAUVideoWidget()
{
    // DELETE THE CAMERA WIDGET IF IT EXISTS
    if (cameraWidget) {
        delete cameraWidget;
    }

    // DELETE THE CAMERA LATER SO THAT IT CAN HANDLE DELETING ITS OWN GLCONTEXTS
    if (camera) {
        camera->deleteLater();
    }

    // DELETE THE CAMERA THREAD AFTER IT SHUTS DOWN
    if (cameraThread) {
        cameraThread->quit();
        while (cameraThread->isRunning()) {
            qApp->processEvents();
        }
        delete cameraThread;
    }

    // DELETE THE CAMERA THREAD AFTER IT SHUTS DOWN
    if (faceDetectorController) {
        delete faceDetectorController;
    } else if (faceGLFilter) {
        delete faceGLFilter;
    }

    // DELETE THE PROJECTOR GLWIDGET
    if (projector) {
        delete projector;
    }

    qDebug() << QString("LAUVideoWidget::~LAUVideoWidget()");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoWidget::showEvent(QShowEvent *)
{
    // SET THE SHARED GLCONTEXT BETWEEN THE GLWIDGET AND CAMERA
    if (glWidget) {
        if (camera && camera->isValid()) {
            // SET THE SHARED CONTEXTED TO THE PROJECTOR
            camera->setShareContext(projector->context());

            // CREATE A FACE FILTER TO DETECT FACES
            faceGLFilter = new LAUFaceDetectorGLFilter(camera->width(), camera->height());

            // SEE IF THE USER LOADED THE FACE DETECTOR FILES
            if (faceGLFilter->hasFaceDetector()) {
                // SET THE SHARED CONTEXTED TO THE PROJECTOR
                faceGLFilter->setShareContext(projector->context());

                // LET THE USER SET THE FACE TEMPLATE
                onSetFaceTemplate();

                // MAKE GLCONTEXT RELATED CONNECTION FROM THE CAMERA TO THE GLWIDGET
                connect(camera, SIGNAL(emitScanFBO(QOpenGLFramebufferObject *)), projector, SLOT(onUpdateProjectorMapping(QOpenGLFramebufferObject *)), Qt::QueuedConnection);
                connect(camera, SIGNAL(emitFaceFBO(QOpenGLFramebufferObject *)), faceGLFilter, SLOT(onUpdateFBO(QOpenGLFramebufferObject *)), Qt::QueuedConnection);
                connect(camera, SIGNAL(emitVideFBO(QOpenGLFramebufferObject *)), glWidget, SLOT(onUpdateVideoBuffer(QOpenGLFramebufferObject *)), Qt::QueuedConnection);

                // NOW MAKE THE CONNECTION FROM FACE DETECTOR TO PROJECTOR WIDGET
                connect(faceGLFilter, SIGNAL(emitFaceVertices(QList<QPointF>)), projector, SLOT(onUpdateFaceVertices(QList<QPointF>)), Qt::QueuedConnection);
                //connect(faceGLFilter, SIGNAL(emitFrameBufferObject(QOpenGLFramebufferObject *)), glWidget, SLOT(onUpdateFaceBuffer(QOpenGLFramebufferObject *)), Qt::QueuedConnection);

                faceDetectorController = new LAUAbstractFilterController(faceGLFilter);
            } else {
                // MAKE GLCONTEXT RELATED CONNECTION FROM THE CAMERA TO THE GLWIDGET
                connect(camera, SIGNAL(emitVideFBO(QOpenGLFramebufferObject *)), glWidget, SLOT(onUpdateVideoBuffer(QOpenGLFramebufferObject *)), Qt::QueuedConnection);
                connect(camera, SIGNAL(emitDeptFBO(QOpenGLFramebufferObject *)), projector, SLOT(onUpdateProjectorMapping(QOpenGLFramebufferObject *)), Qt::QueuedConnection);
            }

            if (QGuiApplication::screens().count() > 1) {
                projector->setGeometry(QGuiApplication::screens().last()->geometry());
                projector->showFullScreen();
            }

            // TELL CAMERA TO START GRABBING
            QTimer::singleShot(1000, camera, SLOT(onEnableVideo()));
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoWidget::insertAction(QAction *action)
{
    QMenu *menu = glWidget->menu();
    if (menu) {
        menu->addAction(action);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoWidget::insertActions(QList<QAction *> actions)
{
    QMenu *menu = glWidget->menu();
    if (menu) {
        menu->addActions(actions);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoWidget::onContextMenuTriggered()
{
    if (cameraWidget) {
        cameraWidget->hide();
        cameraWidget->show();
    } else {
        qDebug() << "LAU3DVideoWidget::onContextMenuTriggered()";
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoWidget::onSetFaceTemplate()
{
    // LOAD THE DEFAULT FACE TEMPLATE
    LAUFacialFeatureTemplateDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        projector->onUpdateFaceTemplate(LAUMemoryObject(dialog.face()), dialog.features());
    }
}
