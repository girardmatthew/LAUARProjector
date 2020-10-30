#include "laukinectglfilter.h"
#include "locale.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUKinectGLFilter::LAUKinectGLFilter(unsigned int dCols, unsigned int dRows, unsigned int cCols, unsigned int cRows, QObject *parent) : LAUAbstractGLFilter(dCols, dRows, parent), rgbCols(cCols), rgbRows(cRows), faceTexture(nullptr), depthTexture(nullptr), frameBufferObjectA(nullptr), frameBufferObjectB(nullptr), frameBufferObjectC(nullptr), frameBufferObjectD(nullptr), textureLookUpTable(nullptr)
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUKinectGLFilter::~LAUKinectGLFilter()
{
    if (wasInitialized() && surface && makeCurrent(surface)) {
        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        if (frameBufferObjectA) {
            delete frameBufferObjectA;
        }
        if (frameBufferObjectB) {
            delete frameBufferObjectB;
        }
        if (frameBufferObjectC) {
            delete frameBufferObjectC;
        }
        if (frameBufferObjectD) {
            delete frameBufferObjectD;
        }
        if (textureLookUpTable) {
            delete textureLookUpTable;
        }
        if (videoTexture) {
            delete videoTexture;
        }
        if (depthTexture) {
            delete depthTexture;
        }
        if (faceTexture) {
            delete faceTexture;
        }

        // RELEASE THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();

        objectA.save(QString("C:/objectA.tif"));
        objectB.save(QString("C:/objectB.tif"));
        objectC.save(QString("C:/objectC.tif"));
        objectD.save(QString("C:/objectD.tif"));
    }
    qDebug() << QString("LAUKinectGLFilter::~LAUKinectGLFilter()");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKinectGLFilter::initialize()
{
    // CREATE A LIST OF FRAME BUFFERS UNTIL WE HAVE ENOUGH
    faceTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    faceTexture->setSize(static_cast<int>(numCols), static_cast<int>(numRows));
    faceTexture->setFormat(QOpenGLTexture::RGBA32F);
    faceTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
    faceTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    faceTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
    faceTexture->allocateStorage();

    depthTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    depthTexture->setSize(static_cast<int>(numCols), static_cast<int>(numRows));
    depthTexture->setFormat(QOpenGLTexture::RGBA32F);
    depthTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
    depthTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    depthTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
    depthTexture->allocateStorage();

    videoTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    videoTexture->setSize(static_cast<int>(rgbCols), static_cast<int>(rgbRows));
    videoTexture->setFormat(QOpenGLTexture::RGBA32F);
    videoTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
    videoTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    videoTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
    videoTexture->allocateStorage();

    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE DEPTH TO COLOR VIDEO MAPPING
    QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);

    // CREATE A FBO TO HOLD THE INTERMEDIATE FFT BUFFERS
    if (portrait()) {
        frameBufferObjectA = new QOpenGLFramebufferObject(static_cast<int>(numRows), static_cast<int>(numCols), frameBufferObjectFormat);
        frameBufferObjectA->release();

        frameBufferObjectC = new QOpenGLFramebufferObject(static_cast<int>(rgbRows), static_cast<int>(rgbCols), frameBufferObjectFormat);
        frameBufferObjectC->release();

        frameBufferObjectD = new QOpenGLFramebufferObject(static_cast<int>(numRows), static_cast<int>(numCols), frameBufferObjectFormat);
        frameBufferObjectD->release();
    } else {
        frameBufferObjectA = new QOpenGLFramebufferObject(static_cast<int>(numCols), static_cast<int>(numRows), frameBufferObjectFormat);
        frameBufferObjectA->release();

        frameBufferObjectC = new QOpenGLFramebufferObject(static_cast<int>(rgbCols), static_cast<int>(rgbRows), frameBufferObjectFormat);
        frameBufferObjectC->release();

        frameBufferObjectD = new QOpenGLFramebufferObject(static_cast<int>(numCols), static_cast<int>(numRows), frameBufferObjectFormat);
        frameBufferObjectD->release();
    }
    frameBufferObjectB = new QOpenGLFramebufferObject(static_cast<int>(numCols), static_cast<int>(numRows), frameBufferObjectFormat);
    frameBufferObjectB->release();

    objectA = LAUMemoryObject(frameBufferObjectA->width(), frameBufferObjectA->height(), 4, sizeof(float));
    objectB = LAUMemoryObject(frameBufferObjectB->width(), frameBufferObjectB->height(), 4, sizeof(float));
    objectC = LAUMemoryObject(frameBufferObjectC->width(), frameBufferObjectC->height(), 4, sizeof(float));
    objectD = LAUMemoryObject(frameBufferObjectD->width(), frameBufferObjectD->height(), 4, sizeof(float));

    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING VIDEO
    setlocale(LC_NUMERIC, "C");
    programA.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/displayGrayVideo.vert");
    programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/displayGrayVideo.frag");
    programA.link();

    programB.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/rawRealSenseVideoToXYZRGBLookUpTable.vert");
    programB.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/rawRealSenseVideoToXYZRGBLookUpTable.frag");
    programB.link();

    programC.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/displayRGBAVideo.vert");
    programC.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/displayRGBAVideo.frag");
    programC.link();

    programD.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/rawRealSenseVideoToXYZRGBNoTable.vert");
    programD.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/rawRealSenseVideoToXYZRGBNoTable.frag");
    programD.link();
    setlocale(LC_ALL, "");

    // LOAD THE LOOK UP TABLE TO THE GPU, IF IT EXISTS
    if (lookUpTable.isValid()) {
        // CREATE TEXTURE FOR HOLDING A 3D LOOK UP TABLE
        if (lookUpTable.style() == StyleXYZPLookUpTable) {
            textureLookUpTable = new QOpenGLTexture(QOpenGLTexture::Target3D);
            textureLookUpTable->setSize(lookUpTable.width(), lookUpTable.height(), lookUpTable.colors() / 4);
            textureLookUpTable->setFormat(QOpenGLTexture::RGBA32F);
            textureLookUpTable->setWrapMode(QOpenGLTexture::ClampToBorder);
            textureLookUpTable->setMinificationFilter(QOpenGLTexture::Linear);
            textureLookUpTable->setMagnificationFilter(QOpenGLTexture::Linear);
            textureLookUpTable->allocateStorage();
            textureLookUpTable->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)lookUpTable.constScanLine(0));
        } else if (lookUpTable.style() == StyleXYZWRCPQLookUpTable) {
            // WE NEED A MEMORY OBJECT TO HOLD THE EXTRACTED DATA
            LAUMemoryObject object(lookUpTable.width(), lookUpTable.height(), 4, sizeof(float), lookUpTable.colors() / 8 - 2);

            // CALCULATE HOW MANY BYTES OF DATA FORM THE MAXIMUM AND
            // MINIMUM LAYERS OF THE USER SUPPLIED LOOK UP TABLE
            unsigned int numFloatsPerImage = lookUpTable.width() * lookUpTable.height() * 8;

            // GET POINTERS TO THE FIRST BYTE OF EACH LOOK UP TABLE
            float *fmBuffer = reinterpret_cast<float *>(lookUpTable.constScanLine(0)) + 2 * numFloatsPerImage;
            float *toBuffer = reinterpret_cast<float *>(object.constScanLine(0));

            // EXTRACT THE PROJECTOR COORDINATES FROM THE USER SUPPLIED LOOK UP TABLE
            int index = 0;
            for (unsigned int frm = 0; frm < object.frames(); frm++) {
                for (unsigned int row = 0; row < object.height(); row++) {
                    for (unsigned int col = 0; col < object.width(); col++) {
                        toBuffer[4 * index + 0] = fmBuffer[8 * index + 4];  // RGB CAMERA COLUMN COORDINATE
                        toBuffer[4 * index + 1] = fmBuffer[8 * index + 5];  // RGB CAMERA ROW COORDINATE
                        toBuffer[4 * index + 2] = fmBuffer[8 * index + 6];  // PROJECTOR COLUMN COORDINATE
                        toBuffer[4 * index + 3] = fmBuffer[8 * index + 7];  // PROJECTOR ROW COORDINATE

                        index++;
                    }
                }
            }

            // UPLOAD THE TEMPORARY MEMORY OBJECT TO THE GPU AS THE LOOK UP TABLE
            textureLookUpTable = new QOpenGLTexture(QOpenGLTexture::Target3D);
            textureLookUpTable->setSize(object.width(), object.height(), object.frames());
            textureLookUpTable->setFormat(QOpenGLTexture::RGBA32F);
            textureLookUpTable->setWrapMode(QOpenGLTexture::ClampToBorder);
            textureLookUpTable->setMinificationFilter(QOpenGLTexture::Linear);
            textureLookUpTable->setMagnificationFilter(QOpenGLTexture::Linear);
            textureLookUpTable->allocateStorage();
            textureLookUpTable->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, (const void *)object.constScanLine(0));
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKinectGLFilter::setLookUpTable(LAULookUpTable lut)
{
    if (lut.isValid()) {
        // KEEP A LOCAL COPY OF THE LOOK UP TABLE
        if (lut.style() == StyleXYZPLookUpTable) {
            lookUpTable = lut;
        } else if (lut.style() == StyleXYZWRCPQLookUpTable) {
            lookUpTable = lut;
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKinectGLFilter::setFace(const unsigned short *buffer)
{
    if (surface && makeCurrent(surface)) {
        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // UPLOAD THE FRAME TO THE GPU
        faceTexture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt16, reinterpret_cast<const void *>(buffer), &options);

        // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
        // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
        if (frameBufferObjectA && frameBufferObjectA->bind()) {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glViewport(0, 0, frameBufferObjectA->width(), frameBufferObjectA->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // BIND OUR PROGRAM VERTEX AND INDICE BUFFERS FOR DRAWING THE FBO
            if (programA.bind()) {
                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // SET THE GRAYSCALE IMAGE TEXTURE ON THE GPU
                        glActiveTexture(GL_TEXTURE0);
                        faceTexture->bind();
                        programA.setUniformValue("qt_texture", 0);

                        // SET THE FLIP AND PORTRAIT FLAGS
                        if (portrait()) {
                            programA.setUniformValue("qt_flip", -1.0f);
                        } else {
                            programA.setUniformValue("qt_flip", 1.0f);
                        }
                        programA.setUniformValue("qt_portrait", portrait());

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
            frameBufferObjectA->release();
        }

        // COPY THE FBO TO THE CPU FOR LATER
        glBindTexture(GL_TEXTURE_2D, frameBufferObjectA->texture());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, objectA.constScanLine(0));

        // RELEASE THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKinectGLFilter::setDepth(const unsigned short *buffer)
{
    if (surface && makeCurrent(surface)) {
        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // UPLOAD THE FRAME TO THE GPU
        depthTexture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt16, reinterpret_cast<const void *>(buffer), &options);

        if (lookUpTable.isValid()) {
            // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
            // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
            if (frameBufferObjectB && frameBufferObjectB->bind()) {
                glClearColor(NAN, NAN, NAN, NAN);
                glViewport(0, 0, frameBufferObjectB->width(), frameBufferObjectB->height());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // BIND OUR PROGRAM VERTEX AND INDICE BUFFERS FOR DRAWING THE FBO
                if (programB.bind()) {
                    if (quadVertexBuffer.bind()) {
                        if (quadIndexBuffer.bind()) {
                            // SET THE ACTIVE TEXTURE ON THE GPU
                            glActiveTexture(GL_TEXTURE0);
                            depthTexture->bind();
                            programB.setUniformValue("qt_dptTexture", 0);

                            // SET THE LUT TEXTURE ON THE GPU
                            if (textureLookUpTable) {
                                glActiveTexture(GL_TEXTURE2);
                                textureLookUpTable->bind();
                                programB.setUniformValue("qt_lutTexture", 2);

                                // SET THE LUT PARAMETERS AS UNIFORM VARIABLES
                                programB.setUniformValue("qt_numLayers", (float)(textureLookUpTable->depth() - 1));
                                programB.setUniformValue("qt_dptLimits", lookUpTable.pLimits());
                            }

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
                frameBufferObjectB->release();

                // COPY THE FBO TO THE CPU FOR LATER
                glBindTexture(GL_TEXTURE_2D, frameBufferObjectB->texture());
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, objectB.constScanLine(0));

                // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
                // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
                if (frameBufferObjectD && frameBufferObjectD->bind()) {
                    glClearColor(NAN, NAN, NAN, NAN);
                    glViewport(0, 0, frameBufferObjectD->width(), frameBufferObjectD->height());
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    // BIND OUR PROGRAM VERTEX AND INDICE BUFFERS FOR DRAWING THE FBO
                    if (programC.bind()) {
                        if (quadVertexBuffer.bind()) {
                            if (quadIndexBuffer.bind()) {
                                // SET THE VIDEO TEXTURE ON THE GPU
                                glActiveTexture(GL_TEXTURE0);
                                glBindTexture(GL_TEXTURE_2D, frameBufferObjectB->texture());
                                programC.setUniformValue("qt_rgbTexture", 0);

                                // SET THE FLIP AND PORTRAIT FLAGS
                                if (portrait()) {
                                    programC.setUniformValue("qt_flip", -1.0f);
                                } else {
                                    programC.setUniformValue("qt_flip", 1.0f);
                                }
                                programC.setUniformValue("qt_portrait", portrait());

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
                    frameBufferObjectD->release();
                }
                // COPY THE FBO TO THE CPU FOR LATER
                glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, objectD.constScanLine(0));
            }
        } else {
            // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
            // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
            if (frameBufferObjectB && frameBufferObjectB->bind()) {
                glClearColor(NAN, NAN, NAN, NAN);
                glViewport(0, 0, frameBufferObjectB->width(), frameBufferObjectB->height());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // BIND OUR PROGRAM VERTEX AND INDICE BUFFERS FOR DRAWING THE FBO
                if (programD.bind()) {
                    if (quadVertexBuffer.bind()) {
                        if (quadIndexBuffer.bind()) {
                            // SET THE DEPTH TEXTURE ON THE GPU
                            glActiveTexture(GL_TEXTURE0);
                            depthTexture->bind();
                            programD.setUniformValue("qt_dptTexture", 0);
                            programD.setUniformValue("qt_dptLimits", QPointF(0.0, 3000.0));

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
                frameBufferObjectB->release();

                // COPY THE FBO TO THE CPU FOR LATER
                glBindTexture(GL_TEXTURE_2D, frameBufferObjectB->texture());
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, objectB.constScanLine(0));

                // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
                // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
                if (frameBufferObjectD && frameBufferObjectD->bind()) {
                    glClearColor(NAN, NAN, NAN, NAN);
                    glViewport(0, 0, frameBufferObjectD->width(), frameBufferObjectD->height());
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    // BIND OUR PROGRAM VERTEX AND INDICE BUFFERS FOR DRAWING THE FBO
                    if (programC.bind()) {
                        if (quadVertexBuffer.bind()) {
                            if (quadIndexBuffer.bind()) {
                                // SET THE VIDEO TEXTURE ON THE GPU
                                glActiveTexture(GL_TEXTURE0);
                                glBindTexture(GL_TEXTURE_2D, frameBufferObjectB->texture());
                                programC.setUniformValue("qt_rgbTexture", 0);

                                // SET THE FLIP AND PORTRAIT FLAGS
                                if (portrait()) {
                                    programC.setUniformValue("qt_flip", -1.0f);
                                } else {
                                    programC.setUniformValue("qt_flip", 1.0f);
                                }
                                programC.setUniformValue("qt_portrait", portrait());

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
                    frameBufferObjectD->release();
                }
                // COPY THE FBO TO THE CPU FOR LATER
                glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, objectD.constScanLine(0));
            }
        }

        // RELEASE THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKinectGLFilter::setVideo(const unsigned char *buffer)
{
    if (surface && makeCurrent(surface)) {
        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // UPLOAD THE FRAME TO THE GPU
        videoTexture->setData(QOpenGLTexture::BGRA, QOpenGLTexture::UInt8, reinterpret_cast<const void *>(buffer), &options);

        // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
        // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
        if (frameBufferObjectC && frameBufferObjectC->bind()) {
            glClearColor(NAN, NAN, NAN, NAN);
            glViewport(0, 0, frameBufferObjectC->width(), frameBufferObjectC->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // BIND OUR PROGRAM VERTEX AND INDICE BUFFERS FOR DRAWING THE FBO
            if (programC.bind()) {
                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // SET THE VIDEO TEXTURE ON THE GPU
                        glActiveTexture(GL_TEXTURE0);
                        videoTexture->bind();
                        programC.setUniformValue("qt_rgbTexture", 0);

                        // SET THE FLIP AND PORTRAIT FLAGS
                        if (portrait()) {
                            programC.setUniformValue("qt_flip", -1.0f);
                        } else {
                            programC.setUniformValue("qt_flip", 1.0f);
                        }
                        programC.setUniformValue("qt_portrait", portrait());

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
            frameBufferObjectC->release();
        }
        // COPY THE FBO TO THE CPU FOR LATER
        glBindTexture(GL_TEXTURE_2D, frameBufferObjectC->texture());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, objectC.constScanLine(0));

        // RELEASE THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKinectGLFilter::scanObject(float *buffer)
{
    if (surface && makeCurrent(surface)) {
        if (frameBufferObjectB) {
            glBindTexture(GL_TEXTURE_2D, frameBufferObjectB->texture());
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);
        }
    }
}
