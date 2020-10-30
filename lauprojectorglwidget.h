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

#ifndef LAUPROJECTORGLWIDGET_H
#define LAUPROJECTORGLWIDGET_H

#include <QMenu>
#include <QScreen>
#include <QWidget>
#include <QObject>
#include <QSettings>
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
#include "laufacedetectorglfilter.h"

#define DISPLAYPROJECTORASPOINTS
#define NUMBERBODYVERTICES         32

class LAUProjectorGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    LAUProjectorGLWidget(unsigned int cols = 0, unsigned int rows = 0, unsigned int frms = 1, QWidget *parent = nullptr);
    ~LAUProjectorGLWidget();

    virtual bool isValid() const
    {
        return (wasInitialized());
    }

    bool wasInitialized() const
    {
        return (vertexArrayObject.isCreated());
    }

public slots:
    void onUpdateFaceVertices(QList<QPointF> points);
    void onUpdateBodyVertices(QList<QPointF> points);
    void onUpdateProjectorMapping(QOpenGLFramebufferObject *object);
    void onUpdateProjectorTexture(QOpenGLFramebufferObject *object);
    void onUpdateFaceTemplate(LAUMemoryObject object, QList<QPointF> points);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    enum Modality { ModeUnknown, ModeScan, ModeFace, ModeBody };

    Modality mode;
    QList<QPointF> facePoints;
    QList<QPointF> bodyPoints;
    QList<QPointF> templatePoints;

    QOpenGLPixelTransferOptions options;
    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLBuffer quadVertexBuffer, quadIndexBuffer;
    QOpenGLBuffer pixelVertexBuffer, pixelIndexBuffer;
    QOpenGLFramebufferObject *scanBufferObject, *faceBufferObject, *bodyBufferObject;
    QOpenGLShaderProgram programA, programB, programC, programD, programE;
    QOpenGLTexture *textureA, *textureB, *textureC, *templateTexture;

    QOpenGLBuffer faceVertexBuffer, faceIndexBuffer;
    QOpenGLBuffer bodyVertexBuffer, bodyIndexBuffer;

    //LAUMemoryObject objectA, objectB;

    bool initializedFlag;
    int localWidth, localHeight;
    unsigned int numCols, numRows, numFrms, numInds;
    unsigned short maxIntensityValue;

signals:
    void emitBuffer(LAUMemoryObject object);
};

#endif // LAUPROJECTORGLWIDGET_H
