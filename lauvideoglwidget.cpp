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
#include "lauvideoglwidget.h"
#include <locale.h>
#include <math.h>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUVideoGLWidget::LAUVideoGLWidget(unsigned int cols, unsigned int rows, unsigned int frms, QWidget *parent) : QOpenGLWidget(parent), numCols(cols), numRows(rows), numFrms(frms), texture(nullptr), faceBufferObject(nullptr), scanBufferObject(nullptr), videoBufferObject(nullptr), contextMenu(nullptr)
{
    mode = ModeFace;
    options.setAlignment(1);
    contextMenu = new QMenu();
    setFocusPolicy(Qt::StrongFocus);
    if (numCols > 0) {
        setMinimumWidth(qMin((int)numCols, 320));
    } else {
        setMinimumWidth(320);
    }
    if (numRows > 0) {
        setMinimumHeight(qMin((int)numCols, 240));
    } else {
        setMinimumHeight(240);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUVideoGLWidget::~LAUVideoGLWidget()
{
    if (wasInitialized()) {
        makeCurrent();
        if (texture) {
            delete texture;
        }
    }

    // DELETE THE CONTEXT MENU IF IT EXISTS
    if (contextMenu) {
        delete contextMenu;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char *)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char *)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char *)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

    // CREATE THE VERTEX ARRAY OBJECT FOR FEEDING VERTICES TO OUR SHADER PROGRAMS
    vertexArrayObject.create();
    vertexArrayObject.bind();

    // CREATE VERTEX BUFFER TO HOLD CORNERS OF QUADRALATERAL
    quadVertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    quadVertexBuffer.create();
    quadVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (quadVertexBuffer.bind()) {
        // ALLOCATE THE VERTEX BUFFER FOR HOLDING THE FOUR CORNERS OF A RECTANGLE
        quadVertexBuffer.allocate(16 * sizeof(float));
        float *buffer = (float *)quadVertexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (buffer) {
            buffer[0]  = -1.0;
            buffer[1]  = -1.0;
            buffer[2]  = 0.0;
            buffer[3]  = 1.0;
            buffer[4]  = +1.0;
            buffer[5]  = -1.0;
            buffer[6]  = 0.0;
            buffer[7]  = 1.0;
            buffer[8]  = +1.0;
            buffer[9]  = +1.0;
            buffer[10] = 0.0;
            buffer[11] = 1.0;
            buffer[12] = -1.0;
            buffer[13] = +1.0;
            buffer[14] = 0.0;
            buffer[15] = 1.0;
            quadVertexBuffer.unmap();
        } else {
            qDebug() << QString("quadVertexBuffer not allocated.") << glGetError();
        }
        quadVertexBuffer.release();
    }

    // CREATE INDEX BUFFER TO ORDERINGS OF VERTICES FORMING POLYGON
    quadIndexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    quadIndexBuffer.create();
    quadIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (quadIndexBuffer.bind()) {
        quadIndexBuffer.allocate(6 * sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)quadIndexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            indices[0] = 0;
            indices[1] = 1;
            indices[2] = 2;
            indices[3] = 0;
            indices[4] = 2;
            indices[5] = 3;
            quadIndexBuffer.unmap();
        } else {
            qDebug() << QString("indiceBufferA buffer mapped from GPU.");
        }
        quadIndexBuffer.release();
    }

    // RELEASE THE VERTEX ARRAY BUFFER
    vertexArrayObject.release();

    // CREATE TEXTURE FOR DISPLAYING NO VIDEO SCREEN
    LAUMemoryObject object(QImage(":/Images/NoVideoScreen.jpg"));
    texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texture->setSize((int)object.width(), (int)object.height());
    texture->setFormat(QOpenGLTexture::RGBA32F);
    texture->setWrapMode(QOpenGLTexture::ClampToBorder);
    texture->setMinificationFilter(QOpenGLTexture::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->allocateStorage();
    texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)object.constPointer(), &options);
    qDebug() << texture->format();

    // CREATE SHADERS THAT FIRST MAP THE INCOMING VIDEO TO THE FRAME BUFFER OBJECT
    // AND THEN JUST DISPLAY THE FBO AS A COLOR TEXTURE.  SO ITS UP TO THE FIRST
    // SHADER TO MAP GRAY TO RGB AND TO PROPERLY SCALE UNSIGNED SHORTS, IF NEEDED.
    setlocale(LC_NUMERIC, "C");

    programA.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/displayPhaseVideo.vert");
    programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/displayPhaseVideo.frag");
    programA.link();

    programB.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/displayRGBAVideo.vert");
    programB.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/displayRGBAVideo.frag");
    programB.link();

    programC.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/displayGrayVideo.vert");
    programC.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/displayGrayVideo.frag");
    programC.link();

    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoGLWidget::resizeGL(int w, int h)
{
    // Get the Desktop Widget so that we can get information about multiple monitors connected to the system.
    QDesktopWidget *dkWidget = QApplication::desktop();
    QList<QScreen *> screenList = QGuiApplication::screens();
    qreal devicePixelRatio = screenList[dkWidget->screenNumber(this)]->devicePixelRatio();
    localHeight = qRound(h * devicePixelRatio);
    localWidth = qRound(w * devicePixelRatio);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoGLWidget::paintGL()
{
    qDebug() << "LAUVideoGLWidget::paintGL()";

    // SET THE VIEW PORT AND CLEAR THE SCREEN BUFFER
    glViewport(0, 0, localWidth, localHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // BIND THE VERTEX ARRAY OBJECT
    vertexArrayObject.bind();

    // MAKE SURE WE HAVE A TEXTURE TO SHOW
    if (scanBufferObject && mode == ModeScan) {
        if (programA.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, scanBufferObject->texture());
                    programA.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    programA.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    programA.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            programA.release();
        }
    } else if (faceBufferObject && mode == ModeFace) {
        if (programC.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, faceBufferObject->texture());
                    programC.setUniformValue("qt_texture", 1);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    programC.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    programC.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            programC.release();
        }
    } else if (videoBufferObject && mode == ModeVideo) {
        if (programB.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, videoBufferObject->texture());
                    programB.setUniformValue("qt_texture", 1);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    programB.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    programB.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            programB.release();
        }
    } else {
        if (programB.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE1);
                    texture->bind();
                    programB.setUniformValue("qt_texture", 1);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    programB.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    programB.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            programB.release();
        }
    }

    // RELEASE THE VERTEX ARRAY BUFFER
    vertexArrayObject.release();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAUVideoGLWidget::grabScreenShot()
{
    // MAKE THIS THE CURRENT OPENGL CONTEXT
    makeCurrent();

    // MAKE SURE WE HAVE A TEXTURE TO SHOW
    if (mode == ModeScan && scanBufferObject) {
        LAUMemoryObject object(static_cast<unsigned int>(scanBufferObject->width()), static_cast<unsigned int>(scanBufferObject->height()), 4, sizeof(float));
        glBindTexture(GL_TEXTURE_2D, faceBufferObject->texture());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, object.constPointer());
        return (object);
    } else if (mode == ModeFace && faceBufferObject) {
        LAUMemoryObject object(static_cast<unsigned int>(faceBufferObject->width()), static_cast<unsigned int>(faceBufferObject->height()), 4, sizeof(float));
        glBindTexture(GL_TEXTURE_2D, faceBufferObject->texture());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, object.constPointer());
        return (object);
    } else if (mode == ModeVideo && videoBufferObject) {
        LAUMemoryObject object(static_cast<unsigned int>(videoBufferObject->width()), static_cast<unsigned int>(videoBufferObject->height()), 4, sizeof(float));
        glBindTexture(GL_TEXTURE_2D, videoBufferObject->texture());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, object.constPointer());
        return (object);
    }
    return (LAUMemoryObject());
}
