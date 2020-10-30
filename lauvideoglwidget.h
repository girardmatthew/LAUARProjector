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

#ifndef LAUVIDEOGLWIDGET_H
#define LAUVIDEOGLWIDGET_H

#include <QMenu>
#include <QScreen>
#include <QWidget>
#include <QObject>
#include <QMatrix4x4>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QCoreApplication>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPixelTransferOptions>

#include "laumemoryobject.h"

class LAUVideoGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    LAUVideoGLWidget(unsigned int cols = 0, unsigned int rows = 0, unsigned int frms = 1, QWidget *parent = nullptr);
    ~LAUVideoGLWidget();

    virtual bool isValid() const
    {
        return (wasInitialized());
    }

    bool wasInitialized() const
    {
        return (vertexArrayObject.isCreated());
    }

    QMenu *menu() const
    {
        return (contextMenu);
    }

    void setMaximumIntensityValue(unsigned short value)
    {
        maxIntensityValue = value;
    }

    LAUMemoryObject grabScreenShot();

public slots:
    void onUpdateScanBuffer(QOpenGLFramebufferObject *fbo)
    {
        scanBufferObject = fbo;
        mode = ModeScan;
        update();
    }

    void onUpdateFaceBuffer(QOpenGLFramebufferObject *fbo)
    {
        faceBufferObject = fbo;
        mode = ModeFace;
        update();
    }

    void onUpdateVideoBuffer(QOpenGLFramebufferObject *fbo)
    {
        videoBufferObject = fbo;
        mode = ModeVideo;
        update();
    }

protected:
    void mousePressEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::RightButton) {
            if (contextMenu) {
                contextMenu->popup(event->globalPos());
            }
        }
    }
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    enum Modality { ModeScan, ModeFace, ModeVideo };

    Modality mode;
    bool initializedFlag;
    int localWidth, localHeight;
    unsigned int numCols, numRows, numFrms;
    unsigned short maxIntensityValue;

    QOpenGLTexture *texture;
    QOpenGLPixelTransferOptions options;
    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLBuffer quadVertexBuffer, quadIndexBuffer;
    QOpenGLFramebufferObject *faceBufferObject, *scanBufferObject, *videoBufferObject;
    QOpenGLShaderProgram programA, programB, programC;
    QMenu *contextMenu;

signals:
    void emitBuffer(LAUMemoryObject object);
};


#endif // LAUVIDEOGLWIDGET_H
