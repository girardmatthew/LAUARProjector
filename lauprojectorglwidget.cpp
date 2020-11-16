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
#include "lauprojectorglwidget.h"
#include <locale.h>
#include <math.h>

#define NUMBEROFFACIALFEATURES    68
#define NUMBEROFFACIALTRIANGLES  107

unsigned int triangles[3 * NUMBEROFFACIALTRIANGLES] = { 67, 58, 59, 31, 32, 49, 58, 57, 7, 52, 34, 35, 45, 44, 25, 40, 39, 29,
                                                        18, 37, 36, 42, 22, 43, 59, 48, 60, 1, 36, 41, 61, 50, 51, 52, 53, 63,
                                                        65, 56, 66, 67, 61, 62, 55, 10, 56, 57, 9, 8, 27, 28, 39, 33, 52, 51,
                                                        53, 55, 65, 65, 66, 62, 42, 28, 27, 35, 46, 14, 20, 37, 19, 19, 37, 18,
                                                        0, 36, 1, 17, 18, 36, 20, 38, 37, 20, 21, 38, 38, 21, 39, 43, 23, 44,
                                                        34, 30, 35, 39, 21, 27, 29, 28, 42, 29, 39, 28, 33, 30, 34, 29, 31, 40,
                                                        30, 29, 35, 30, 32, 31, 17, 36, 0, 49, 48, 31, 30, 31, 29, 41, 2, 1, 48,
                                                        3, 2, 31, 48, 2, 59, 58, 6, 48, 4, 3, 48, 5, 4, 48, 59, 5, 5, 59, 6, 67,
                                                        66, 58, 58, 7, 6, 60, 49, 59, 33, 32, 30, 59, 49, 67, 60, 48, 49, 41, 31,
                                                        2, 41, 40, 31, 49, 61, 67, 49, 32, 50, 33, 50, 32, 61, 49, 50, 63, 62,
                                                        51, 62, 61, 51, 52, 63, 51, 66, 67, 62, 35, 53, 52, 65, 62, 63, 66, 57,
                                                        58, 56, 9, 57, 57, 8, 7, 66, 56, 57, 65, 63, 53, 65, 55, 56, 53, 64, 55,
                                                        56, 10, 9, 55, 11, 10, 54, 64, 53, 64, 54, 55, 35, 54, 53, 54, 11, 55, 14,
                                                        54, 35, 13, 12, 54, 54, 12, 11, 47, 29, 42, 54, 14, 13, 46, 15, 14, 52,
                                                        33, 34, 51, 50, 33, 22, 42, 27, 47, 35, 29, 22, 23, 43, 46, 35, 47, 21, 22,
                                                        27, 24, 44, 23, 25, 44, 24, 16, 45, 26, 15, 46, 45, 45, 25, 26, 45, 16, 15
                                                      };

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUProjectorGLWidget::LAUProjectorGLWidget(unsigned int cols, unsigned int rows, unsigned int frms, QWidget *parent) : QOpenGLWidget(parent), mode(ModeUnknown), numCols(cols), numRows(rows), numFrms(frms), numInds(0), textureA(nullptr), textureB(nullptr), textureC(nullptr), templateTexture(nullptr), scanBufferObject(nullptr), faceBufferObject(nullptr), bodyBufferObject(nullptr)
{
    options.setAlignment(1);
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
LAUProjectorGLWidget::~LAUProjectorGLWidget()
{
    if (wasInitialized()) {
        makeCurrent();

        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // FREE THE ALLOCATED BUFFERS
        if (textureA) {
            delete textureA;
        }

        if (textureB) {
            delete textureB;
        }

        if (textureC) {
            delete textureC;
        }

        if (templateTexture) {
            delete templateTexture;
        }

        if (scanBufferObject) {
            delete scanBufferObject;
        }

        if (faceBufferObject) {
            delete faceBufferObject;
        }

        if (bodyBufferObject){
            delete bodyBufferObject;
        }

        // RELEASE THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();

        //objectA.save(QString("C:/objectAAA.tif"));
        //objectB.save(QString("C:/objectBBB.tif"));
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUProjectorGLWidget::onUpdateFaceVertices(QList<QPointF> points)
{
    // SET THE MODE TO DRAWING A FACE
    mode = ModeFace;
    facePoints = points;

    // COPY FACE TRIANGLE VERTICES TO THE GPU FOR DRAWING
    int numPoints = qMin(facePoints.size(), templatePoints.size());

    if (numPoints == NUMBEROFFACIALFEATURES) {
        // MAKE THIS GLWIDGET THE CURRENT CONTEXT FOR DRAWING
        makeCurrent();

        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        if (faceVertexBuffer.bind()) {
            float *buffer = (float *)faceVertexBuffer.mapRange(0, NUMBEROFFACIALFEATURES * 4 * sizeof(float), QOpenGLBuffer::RangeWrite);
            if (buffer) {
                for (unsigned int n = 0; n < (unsigned int)NUMBEROFFACIALFEATURES; n++) {
                    // INSERT FIRST OF THREE VERTICES (INPUT AND OUTPUT POINTS)
                    buffer[4 * n + 0] = facePoints.at(n).x();
                    buffer[4 * n + 1] = facePoints.at(n).y();
                    buffer[4 * n + 2] = templatePoints.at(n).x();
                    buffer[4 * n + 3] = templatePoints.at(n).y();
                }
                faceVertexBuffer.unmap();
            } else {
                qDebug() << QString("faceVertexBuffer not mapped to CPU.") << glGetError();
            }
            faceVertexBuffer.release();

            // MAKE SURE WE HAVE A VALID FBO TO DRAW INTO
            if (faceBufferObject && faceBufferObject->bind()) {
                if (programB.bind()) {
                    // SET THE VIEWPOINT AND CLEAR THE PREVIOUS CONTENTS OF THE BUFFER
                    glViewport(0, 0, faceBufferObject->width(), faceBufferObject->height());
                    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                    if (faceVertexBuffer.bind()) {
                        if (faceIndexBuffer.bind()) {
                            // BIND THE INCOMING RGB VIDEO FRAME AS A TEXTURE
                            glActiveTexture(GL_TEXTURE2);
                            templateTexture->bind();
                            programB.setUniformValue("qt_texture", 2);

                            // BIND THE SCAN BUFFER SO WE HAVE A MAPPING
                            glActiveTexture(GL_TEXTURE4);
                            glBindTexture(GL_TEXTURE_2D, scanBufferObject->texture());
                            programB.setUniformValue("qt_mapping", 4);

                            // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                            glVertexAttribPointer(programB.attributeLocation("qt_vertex"), 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
                            programB.enableAttributeArray("qt_vertex");

                            // DRAW THE TRIANGLES CONNECTING FACIAL FEATURES
                            glDrawElements(GL_TRIANGLES, 3 * NUMBEROFFACIALTRIANGLES, GL_UNSIGNED_INT, nullptr);

                            // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                            faceIndexBuffer.release();
                        }
                        faceVertexBuffer.release();
                    }
                    programB.release();
                }
                faceBufferObject->release();
            }
            //glBindTexture(GL_TEXTURE_2D, faceBufferObject->texture());
            //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, objectB.constPointer());

            // RELEASE THE VERTEX ARRAY OBJECT
            vertexArrayObject.release();

            // TELL THE UNDERLYING WIDGET TO REDRAW ITSELF
            update();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUProjectorGLWidget::onUpdateBodyVertices(QList<QPointF> points)
{
    // SET THE MODE TO DRAWING A BODY
    mode = ModeBody;
    bodyPoints = points;

    // COPY BODY TRIANGLE VERTICES TO THE GPU FOR DRAWING
    int numPoints = bodyPoints.size();

    if (numPoints == NUMBERBODYVERTICES) {
        // MAKE THIS GLWIDGET THE CURRENT CONTEXT FOR DRAWING
        makeCurrent();

        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        if (bodyVertexBuffer.bind()) {
            float *buffer = (float *)bodyVertexBuffer.mapRange(0, NUMBERBODYVERTICES * 4 * sizeof(float), QOpenGLBuffer::RangeWrite);
            if (buffer) {
                for (unsigned int n = 0; n < (unsigned int)NUMBERBODYVERTICES; n++) {
                    // INSERT FIRST OF THREE VERTICES (INPUT AND OUTPUT POINTS)
                    buffer[4 * n + 0] = bodyPoints.at(n).x();
                    buffer[4 * n + 1] = bodyPoints.at(n).y();
                    buffer[4 * n + 2] = templatePoints.at(n).x();
                    buffer[4 * n + 3] = templatePoints.at(n).y();
                }
                bodyVertexBuffer.unmap();
            } else {
                qDebug() << QString("bodyVertexBuffer not mapped to CPU.") << glGetError();
            }
            bodyVertexBuffer.release();

            // MAKE SURE WE HAVE A VALID FBO TO DRAW INTO
            if (bodyBufferObject && bodyBufferObject->bind()) {
                if (programB.bind()) {
                    // SET THE VIEWPOINT AND CLEAR THE PREVIOUS CONTENTS OF THE BUFFER
                    glViewport(0, 0, bodyBufferObject->width(), bodyBufferObject->height());
                    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
                    if (bodyVertexBuffer.bind()) {
                        if (bodyIndexBuffer.bind()) {
                            // BIND THE INCOMING RGB VIDEO FRAME AS A TEXTURE
                            glActiveTexture(GL_TEXTURE2);
                            templateTexture->bind();
                            programB.setUniformValue("qt_texture", 2);

                            // BIND THE SCAN BUFFER SO WE HAVE A MAPPING
                            glActiveTexture(GL_TEXTURE4);
                            glBindTexture(GL_TEXTURE_2D, scanBufferObject->texture());
                            programB.setUniformValue("qt_mapping", 4);

                            // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                            glVertexAttribPointer(programB.attributeLocation("qt_vertex"), 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
                            programB.enableAttributeArray("qt_vertex");

                            // DRAW THE TRIANGLES CONNECTING FACIAL FEATURES
                            glDrawElements(GL_TRIANGLES, 3 * NUMBEROFFACIALTRIANGLES, GL_UNSIGNED_INT, nullptr);

                            // RELEASE THE FRAME BUFFER OBJECT AND ITS ASSOCIATED GLSL PROGRAMS
                            bodyIndexBuffer.release();
                        }
                        bodyVertexBuffer.release();
                    }
                    programB.release();
                }
                bodyBufferObject->release();
            }
            //glBindTexture(GL_TEXTURE_2D, bodyBufferObject->texture());
            //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, objectB.constPointer());

            // RELEASE THE VERTEX ARRAY OBJECT
            vertexArrayObject.release();

            // TELL THE UNDERLYING WIDGET TO REDRAW ITSELF
            update();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUProjectorGLWidget::onUpdateProjectorMapping(QOpenGLFramebufferObject *object)
{
    // MAKE SURE WE HAVE A VALID SOURCE AND TARGET FRAME BUFFER OBJECT
    if (object && scanBufferObject) {
        // CHANGE THE STATE OF THE VIDEO DISPLAY TO SCAN MODE
        if (mode != ModeFace) {
            mode = ModeScan;
        }

        // MAKE THIS THE CURRENT OPENGL CONTEXT
        makeCurrent();

        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // MAKE SURE WE HAVE A VALID FBO TO DRAW INTO
        if (scanBufferObject->bind()) {
            if (programD.bind()) {
                // SET THE VIEWPOINT BUT DON'T CLEAR THE PREVIOUS CONTENTS OF THE BUFFER
                glViewport(0, 0, scanBufferObject->width(), scanBufferObject->height());
                glClearColor(NAN, NAN, NAN, NAN);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // SET THE ACTIVE TEXTURE ON THE GPU
                        glActiveTexture(GL_TEXTURE3);
                        glBindTexture(GL_TEXTURE_2D, object->texture());
                        programD.setUniformValue("qt_texture", 3);
                        programD.setUniformValue("qt_portrait", false);
                        programD.setUniformValue("qt_flip", 1.0f);

                        // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                        programD.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                        programD.enableAttributeArray("qt_vertex");

                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                        quadIndexBuffer.release();
                    }
                    quadVertexBuffer.release();
                }
                programD.release();
            }
            scanBufferObject->release();
        }
        //glBindTexture(GL_TEXTURE_2D, scanBufferObject->texture());
        //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, objectA.constPointer());

        // RELEASE THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();

        // TELL THE UNDERLYING WIDGET TO REDRAW ITSELF
        update();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUProjectorGLWidget::onUpdateProjectorTexture(QOpenGLFramebufferObject *object)
{
    // MAKE SURE WE HAVE A VALID SOURCE AND TARGET FRAME BUFFER OBJECT
    if (object && faceBufferObject) {
        // CHANGE THE STATE OF THE VIDEO DISPLAY TO SCAN MODE
        mode = ModeFace;

        // MAKE THIS THE CURRENT OPENGL CONTEXT
        makeCurrent();

        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // MAKE SURE WE HAVE A VALID FBO TO DRAW INTO
        if (faceBufferObject->bind()) {
            if (programD.bind()) {
                // SET THE VIEWPOINT BUT DON'T CLEAR THE PREVIOUS CONTENTS OF THE BUFFER
                glViewport(0, 0, faceBufferObject->width(), faceBufferObject->height());
                glClearColor(NAN, NAN, NAN, NAN);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // SET THE ACTIVE TEXTURE ON THE GPU
                        glActiveTexture(GL_TEXTURE6);
                        glBindTexture(GL_TEXTURE_2D, object->texture());
                        programD.setUniformValue("qt_texture", 6);

                        // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                        programD.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                        programD.enableAttributeArray("qt_vertex");

                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                        quadIndexBuffer.release();
                    }
                    quadVertexBuffer.release();
                }
                programD.release();
            }
            faceBufferObject->release();
        }

        // RELEASE THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();

        // TELL THE UNDERLYING WIDGET TO REDRAW ITSELF
        update();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUProjectorGLWidget::onUpdateFaceTemplate(LAUMemoryObject object, QList<QPointF> points)
{
    // KEEP A LOCAL COPY OF THE FIDUCIAL POINTS
    templatePoints = points;

    if (object.isValid()) {
        // MAKE THIS THE CURRENT OPENGL CONTEXT
        makeCurrent();

        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // DELETE ANY PREVIOUS TEMPLATE
        if (templateTexture) {
            delete templateTexture;
        }

        // CREATE A NEW TEMPLATE TO HOLD THE INCOMING MEMORY OBJECT
        templateTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        templateTexture->setSize((int)object.width(), (int)object.height());
        templateTexture->setFormat(QOpenGLTexture::RGBA32F);
        templateTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
        templateTexture->setMinificationFilter(QOpenGLTexture::Linear);
        templateTexture->setMagnificationFilter(QOpenGLTexture::Linear);
        templateTexture->allocateStorage();

        if (object.colors() == 1) {
            if (object.depth() == sizeof(unsigned char)) {
                templateTexture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, (const void *)object.constPointer());
            } else if (object.depth() == sizeof(unsigned short)) {
                templateTexture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt16, (const void *)object.constPointer());
            } else if (object.depth() == sizeof(float)) {
                templateTexture->setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, (const void *)object.constPointer());
            }
        } else if (object.colors() == 3) {
            if (object.depth() == sizeof(unsigned char)) {
                templateTexture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, (const void *)object.constPointer());
            } else if (object.depth() == sizeof(unsigned short)) {
                templateTexture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt16, (const void *)object.constPointer());
            } else if (object.depth() == sizeof(float)) {
                templateTexture->setData(QOpenGLTexture::RGB, QOpenGLTexture::Float32, (const void *)object.constPointer());
            }
        } else if (object.colors() == 4) {
            if (object.depth() == sizeof(unsigned char)) {
                templateTexture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, (const void *)object.constPointer());
            } else if (object.depth() == sizeof(unsigned short)) {
                templateTexture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt16, (const void *)object.constPointer());
            } else if (object.depth() == sizeof(float)) {
                templateTexture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)object.constPointer());
            }
        }

        // RELEASE THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUProjectorGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

    // get context opengl-version
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

    // CREATE VERTEX BUFFER TO HOLD FACIAL FEATURE TRIANGLES
    faceVertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    faceVertexBuffer.create();
    faceVertexBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    if (faceVertexBuffer.bind()) {
        faceVertexBuffer.allocate(NUMBEROFFACIALFEATURES * 4 * sizeof(float));
        faceVertexBuffer.release();
    }

    // CREATE INDEX BUFFER TO CONNECT VERTICES FOR FACIAL FEATURE TRIANGLES
    faceIndexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    faceIndexBuffer.create();
    faceIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (faceIndexBuffer.bind()) {
        faceIndexBuffer.allocate(3 * NUMBEROFFACIALTRIANGLES * sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)faceIndexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            memcpy(indices, triangles, 3 * NUMBEROFFACIALTRIANGLES * sizeof(unsigned int));
            faceIndexBuffer.unmap();
        } else {
            qDebug() << QString("indiceBufferA buffer mapped from GPU.");
        }
        faceIndexBuffer.release();
    }

    // CREATE VERTEX BUFFER TO HOLD FACIAL FEATURE TRIANGLES
    bodyVertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    bodyVertexBuffer.create();
    bodyVertexBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    if (bodyVertexBuffer.bind()) {
        bodyVertexBuffer.allocate(NUMBERBODYVERTICES * 4 * sizeof(float));
        bodyVertexBuffer.release();
    }

    // CREATE INDEX BUFFER TO CONNECT VERTICES FOR FACIAL FEATURE TRIANGLES
    bodyIndexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    bodyIndexBuffer.create();
    bodyIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (bodyIndexBuffer.bind()) {
        bodyIndexBuffer.allocate(3 * NUMBERBODYVERTICES * sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)bodyIndexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
            memcpy(indices, triangles, 3 * NUMBERBODYVERTICES * sizeof(unsigned int));
            bodyIndexBuffer.unmap();
        } else {
            qDebug() << QString("indiceBufferA buffer mapped from GPU.");
        }
        bodyIndexBuffer.release();
    }

    // CREATE A BUFFER TO HOLD THE ROW AND COLUMN COORDINATES OF IMAGE PIXELS FOR THE TEXEL FETCHES
    pixelVertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    pixelVertexBuffer.create();
    pixelVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (pixelVertexBuffer.bind()) {
        pixelVertexBuffer.allocate(numRows * numCols * 2 * sizeof(float));
        float *vertices = (float *)pixelVertexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (vertices) {
            for (unsigned int row = 0; row < numRows; row++) {
                for (unsigned int col = 0; col < numCols; col++) {
                    vertices[2 * (col + row * numCols) + 0] = col;
                    vertices[2 * (col + row * numCols) + 1] = row;
                }
            }
            pixelVertexBuffer.unmap();
        } else {
            qDebug() << QString("Unable to map vertexBuffer from GPU.");
        }
    }

    // CREATE AN INDEX BUFFER FOR THE RESULTING POINT CLOUD DRAWN AS TRIANGLES
    numInds = 0;
    pixelIndexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    pixelIndexBuffer.create();
    pixelIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (pixelIndexBuffer.bind()) {
        pixelIndexBuffer.allocate((numRows * numCols * 6)*sizeof(unsigned int));
        unsigned int *indices = (unsigned int *)pixelIndexBuffer.map(QOpenGLBuffer::WriteOnly);
        if (indices) {
#ifdef DISPLAYPROJECTORASPOINTS
            for (unsigned int row = 0; row < numRows; row++) {
                for (unsigned int col = 0; col < numCols; col++) {
                    indices[numInds++] = row * numCols + col;
                }
            }
#else
            for (unsigned int row = 0; row < numRows - 1; row++) {
                for (unsigned int col = 0; col < numCols - 1; col++) {
                    indices[numInds++] = (row + 0) * numCols + (col + 0);
                    indices[numInds++] = (row + 0) * numCols + (col + 1);
                    indices[numInds++] = (row + 1) * numCols + (col + 1);

                    indices[numInds++] = (row + 0) * numCols + (col + 0);
                    indices[numInds++] = (row + 1) * numCols + (col + 1);
                    indices[numInds++] = (row + 1) * numCols + (col + 0);
                }
            }
#endif
            pixelIndexBuffer.unmap();
        } else {
            qDebug() << QString("Unable to map indiceBuffer from GPU.");
        }
    }

    // CREATE A FORMAT OBJECT FOR CREATING THE FRAME BUFFER
    QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);

    // CREATE NEW FRAME BUFFER OBJECTS
    scanBufferObject = new QOpenGLFramebufferObject(numCols, numRows, frameBufferObjectFormat);
    scanBufferObject->release();

    faceBufferObject = new QOpenGLFramebufferObject(numCols, numRows, frameBufferObjectFormat);
    faceBufferObject->release();

    bodyBufferObject = new QOpenGLFramebufferObject(numCols, numRows, frameBufferObjectFormat);
    bodyBufferObject->release();

    //objectA = LAUMemoryObject(scanBufferObject->width(), scanBufferObject->height(), 4, sizeof(float));
    //objectB = LAUMemoryObject(faceBufferObject->width(), faceBufferObject->height(), 4, sizeof(float));

    // CREATE TEXTURE FOR DISPLAYING NO VIDEO SCREEN
    LAUMemoryObject object(QImage(":/Images/NoVideoScreen.jpg"));
    textureA = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureA->setSize((int)object.width(), (int)object.height());
    textureA->setFormat(QOpenGLTexture::RGBA32F);
    textureA->setWrapMode(QOpenGLTexture::ClampToBorder);
    textureA->setMinificationFilter(QOpenGLTexture::Linear);
    textureA->setMagnificationFilter(QOpenGLTexture::Linear);
    textureA->allocateStorage();
    textureA->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)object.constPointer(), &options);

    // CREATE SHADERS THAT FIRST MAP THE INCOMING VIDEO TO THE FRAME BUFFER OBJECT
    // AND THEN JUST DISPLAY THE FBO AS A COLOR TEXTURE.  SO ITS UP TO THE FIRST
    // SHADER TO MAP GRAY TO RGB AND TO PROPERLY SCALE UNSIGNED SHORTS, IF NEEDED.
    setlocale(LC_NUMERIC, "C");

    programB.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/filterDrawFace.vert");
    programB.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/filterDrawFace.frag");
    programB.link();

    programC.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/generateProjectorToCameraMapping.vert");
    programC.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/generateProjectorToCameraMapping.frag");
    programC.link();

    programD.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/displayRGBAVideo.vert");
    programD.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/displayRGBAVideo.frag");
    programD.link();

    programE.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/displayPointCloud.vert");
    programE.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/displayPointCloud.frag");
    programE.link();

    setlocale(LC_ALL, "");

    // RELEASE THE VERTEX ARRAY OBJECT
    vertexArrayObject.release();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUProjectorGLWidget::resizeGL(int w, int h)
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
void LAUProjectorGLWidget::paintGL()
{
    qDebug() << "LAUProjectorGLWidget::paintGL()";

    // SET THE VIEW PORT AND CLEAR THE SCREEN BUFFER
    glViewport(0, 0, localWidth, localHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // BIND THE VERTEX ARRAY OBJECT
    vertexArrayObject.bind();

    if (mode == ModeUnknown) {
        if (programD.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE0);
                    textureA->bind();
                    programD.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    programD.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    programD.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            programD.release();
        }
    } else if (mode == ModeFace) {
        if (programD.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE5);
                    glBindTexture(GL_TEXTURE_2D, faceBufferObject->texture());
                    programD.setUniformValue("qt_texture", 5);
                    programD.setUniformValue("qt_portrait", false);
                    programD.setUniformValue("qt_flip", -1.0f);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    programD.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    programD.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            programD.release();
        }
    } else if (mode == ModeBody) {
        if (programD.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE5);
                    glBindTexture(GL_TEXTURE_2D, faceBufferObject->texture());
                    programD.setUniformValue("qt_texture", 5);
                    programD.setUniformValue("qt_portrait", false);
                    programD.setUniformValue("qt_flip", -1.0f);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    programD.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    programD.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            programD.release();
        }
    } else if (mode == ModeScan) {
        if (programD.bind()) {
            if (quadVertexBuffer.bind()) {
                if (quadIndexBuffer.bind()) {
                    // SET THE ACTIVE TEXTURE ON THE GPU
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, scanBufferObject->texture());
                    programD.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                    programD.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                    programD.enableAttributeArray("qt_vertex");

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                    quadIndexBuffer.release();
                }
                quadVertexBuffer.release();
            }
            programD.release();
        }
    } else {
        // ENABLE THE DEPTH FILTER
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glPointSize(10.0f);

        // BIND THE GLSL PROGRAMS RESPONSIBLE FOR CONVERTING OUR FRAME BUFFER
        // OBJECT TO AN XYZ+TEXTURE POINT CLOUD FOR DISPLAY ON SCREEN
        if (programE.bind()) {
            glViewport(0, 0, localWidth, localHeight);

            // BIND VBOS FOR DRAWING TRIANGLES ON SCREEN
            if (pixelVertexBuffer.bind()) {
                if (pixelIndexBuffer.bind()) {
                    // BIND THE TEXTURE FROM THE FRAME BUFFER OBJECT
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, scanBufferObject->texture());
                    programE.setUniformValue("qt_texture", 0);

                    // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA

                }
                pixelVertexBuffer.release();
            }
            programE.release();
        }
    }

    // RELEASE THE VERTEX ARRAY OBJECT
    vertexArrayObject.release();
}
