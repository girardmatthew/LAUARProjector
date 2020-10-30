#include "laubaslerglfilter.h"
#include "locale.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUBaslerGLFilter::LAUBaslerGLFilter(unsigned int cols, unsigned int rows, QObject *parent) : LAUAbstractGLFilter(cols, rows, parent),
    snrThreshold(200), mtnThreshold(900), frameTexture(nullptr), textureMin(nullptr), textureMax(nullptr), textureLookUpTable(nullptr),
    frameBufferObjectA(nullptr), frameBufferObjectB(nullptr), frameBufferObjectC(nullptr),
    frameBufferObjectD(nullptr), frameBufferObjectE(nullptr), frameBufferObjectF(nullptr)
{
    // CREATE A MEMORY OBJECT FOR TRANSFERING FBOS TO DISK AS TIFF FILES
    localObject = LAUMemoryObject(numCols, numRows, 4, sizeof(float), 1);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUBaslerGLFilter::~LAUBaslerGLFilter()
{
    if (wasInitialized() && surface && makeCurrent(surface)) {
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
        if (frameBufferObjectE) {
            delete frameBufferObjectE;
        }
        if (frameBufferObjectF) {
            delete frameBufferObjectF;
        }
        if (frameBufferObjectG) {
            delete frameBufferObjectG;
        }
        if (frameTexture) {
            delete frameTexture;
        }
        if (textureMin) {
            delete textureMin;
        }
        if (textureMax) {
            delete textureMax;
        }
        if (textureLookUpTable) {
            delete textureLookUpTable;
        }
    }
    qDebug() << QString("LAUBaslerGLFilter::~LAUBaslerGLFilter()");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUBaslerGLFilter::initialize()
{
    // CREATE A LIST OF FRAME BUFFERS UNTIL WE HAVE ENOUGH
    frameTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    frameTexture->setSize(static_cast<int>(numCols), static_cast<int>(numRows));
    frameTexture->setFormat(QOpenGLTexture::RGBA32F);
    frameTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
    frameTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    frameTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
    frameTexture->allocateStorage();

    // CREATE THE GPU SIDE TEXTURE BUFFER TO HOLD THE DEPTH TO COLOR VIDEO MAPPING
    QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
    frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);

    // CREATE A FBO TO HOLD THE INTERMEDIATE FFT BUFFERS
    frameBufferObjectA = new QOpenGLFramebufferObject(static_cast<int>(numCols), static_cast<int>(numRows), frameBufferObjectFormat);
    frameBufferObjectA->release();

    // CREATE A FBO TO HOLD THE FINAL FFT COEFFICIENTS
    frameBufferObjectB = new QOpenGLFramebufferObject(static_cast<int>(numCols), static_cast<int>(numRows), frameBufferObjectFormat);
    frameBufferObjectB->release();

    // CREATE A FBO TO HOLD THE GAUSSIAN SMOOTHED FFT COEFFICIENTS
    frameBufferObjectC = new QOpenGLFramebufferObject(static_cast<int>(numCols), static_cast<int>(numRows), frameBufferObjectFormat);
    frameBufferObjectC->release();

    // CREATE A FBO TO HOLD THE INTERMEDIATE FFT BUFFERS
    frameBufferObjectD = new QOpenGLFramebufferObject(static_cast<int>(numCols), static_cast<int>(numRows), frameBufferObjectFormat);
    frameBufferObjectD->release();

    // CREATE A FBO TO HOLD THE FINAL FFT COEFFICIENTS
    frameBufferObjectE = new QOpenGLFramebufferObject(static_cast<int>(numCols), static_cast<int>(numRows), frameBufferObjectFormat);
    frameBufferObjectE->release();

    // CREATE A FBO TO HOLD THE GAUSSIAN SMOOTHED FFT COEFFICIENTS
    frameBufferObjectF = new QOpenGLFramebufferObject(2 * static_cast<int>(numCols), static_cast<int>(numRows), frameBufferObjectFormat);
    frameBufferObjectF->release();

    // CREATE A FBO TO HOLD THE GAUSSIAN SMOOTHED FFT COEFFICIENTS
    frameBufferObjectG = new QOpenGLFramebufferObject(2 * static_cast<int>(numCols), static_cast<int>(numRows), frameBufferObjectFormat);
    frameBufferObjectG->release();

    // CREATE A FBO TO HOLD THE GAUSSIAN SMOOTHED FFT COEFFICIENTS
    frameBufferObjectH = new QOpenGLFramebufferObject(static_cast<int>(numCols), static_cast<int>(numRows), frameBufferObjectFormat);
    frameBufferObjectH->release();

    // CREATE GLSL PROGRAM FOR PROCESSING THE INCOMING VIDEO
    setlocale(LC_NUMERIC, "C");
    programA.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/addTextureToFBO.vert");
    programA.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/addTextureToFBO.frag");
    programA.link();

    programB.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/generateMagnitudePhase.vert");
    programB.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/generateMagnitudePhase.frag");
    programB.link();

    programC.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/generateXYZG.vert");
    programC.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/generateXYZG.frag");
    programC.link();

    programD.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/processRGGBVideo.vert");
    programD.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/processRGGBVideo.frag");
    programD.link();
    setlocale(LC_ALL, "");

    // LOAD THE LOOK UP TABLE TO THE GPU, IF IT EXISTS
    if (lookUpTable.isValid()) {
        setLookUpTable(lookUpTable);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUBaslerGLFilter::setLookUpTable(LAULookUpTable lut)
{
    if (lut.isValid() && lut.style() == StyleXYZPLookUpTable) {
        // KEEP A LOCAL COPY OF THE LOOK UP TABLE
        lookUpTable = lut;

        // CHECK TO SEE IF WE CAN COPY THIS LOOK UP TABLE TO THE GPU
        if (surface && makeCurrent(surface)) {
            // KEEP TRACK OF THE LOOK UP TABLE'S FIRST BYTE ADDRESS
            float *buffer = reinterpret_cast<float *>(lookUpTable.constScanLine(0));

            // CREATE TEXTURE FOR HOLDING A MINIMUM PHASE VALUES
            textureMin = new QOpenGLTexture(QOpenGLTexture::Target2D);
            textureMin->setSize(static_cast<int>(numCols), static_cast<int>(numRows));
            textureMin->setFormat(QOpenGLTexture::RGBA32F);
            textureMin->allocateStorage();
            textureMin->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, reinterpret_cast<const void *>(buffer + 0 * static_cast<int>(numCols) * static_cast<int>(numRows)));

            // CREATE TEXTURE FOR HOLDING A MINIMUM PHASE VALUES
            textureMax = new QOpenGLTexture(QOpenGLTexture::Target2D);
            textureMax->setSize(static_cast<int>(numCols), static_cast<int>(numRows));
            textureMax->setFormat(QOpenGLTexture::RGBA32F);
            textureMax->allocateStorage();
            textureMax->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, reinterpret_cast<const void *>(buffer + 4 * static_cast<int>(numCols) * static_cast<int>(numRows)));

            // CREATE TEXTURE FOR HOLDING A 3D LOOK UP TABLE
            textureLookUpTable = new QOpenGLTexture(QOpenGLTexture::Target3D);
            textureLookUpTable->setSize(static_cast<int>(numCols), static_cast<int>(numRows), (lookUpTable.colors() - 8) / 4);
            textureLookUpTable->setFormat(QOpenGLTexture::RGBA32F);
            textureLookUpTable->setWrapMode(QOpenGLTexture::ClampToBorder);
            textureLookUpTable->setMinificationFilter(QOpenGLTexture::Linear);
            textureLookUpTable->setMagnificationFilter(QOpenGLTexture::Linear);
            textureLookUpTable->allocateStorage();
            textureLookUpTable->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, reinterpret_cast<const void *>(buffer + 8 * static_cast<int>(numCols) * static_cast<int>(numRows)));
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUBaslerGLFilter::setFace(unsigned short *buffer)
{
    if (surface && makeCurrent(surface)) {
        // UPLOAD THE FRAME TO THE GPU
        frameTexture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt16, reinterpret_cast<const void *>(buffer), &options);

        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
        // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
        if (frameBufferObjectH && frameBufferObjectH->bind()) {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glViewport(0, 0, frameBufferObjectA->width(), frameBufferObjectA->height());

            // BIND OUR PROGRAM VERTEX AND INDICE BUFFERS FOR DRAWING THE FBO
            if (programD.bind()) {
                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // SET THE ACTIVE TEXTURE ON THE GPU
                        glActiveTexture(GL_TEXTURE0);
                        frameTexture->bind();
                        programD.setUniformValue("qt_texture", 0);

                        // SET THE SCALE FACTOR FOR 12 OUT OF 16 BITS PER PIXEL
                        programD.setUniformValue("qt_scaleFactor", 1024.0f);

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
            frameBufferObjectH->release();
        }

        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUBaslerGLFilter::setFrame(unsigned short *buffer, int frm)
{
    if (surface && makeCurrent(surface)) {
        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // UPLOAD THE FRAME TO THE GPU
        frameTexture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt16, reinterpret_cast<const void *>(buffer), &options);

        // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
        // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
        if (frameBufferObjectA && frameBufferObjectA->bind()) {
            // ENABLE ALPHA BLENDING
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);

            // CREATE A SCALAR TO HOLD THE SCALE FACTOR FOR THE FFT COEFFICIENTS
            QVector4D scalar(16.0f, 16.0f, 16.0f, 16.0f);

            // ON THE FIRST FRAME, CLEAR THE FRAME BUFFER OBJECT
            switch (frm % 8) {
                case 0:
                    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                    glViewport(0, 0, frameBufferObjectA->width(), frameBufferObjectA->height());
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    scalar *= QVector4D(static_cast<float>(cos(2.0 * PI * 0.0 / 8.0)), static_cast<float>(sin(2.0 * PI * 0.0 / 8.0)), static_cast<float>(cos(4.0 * PI * 0.0 / 8.0)), static_cast<float>(sin(4.0 * PI * 0.0 / 8.0)));
                    break;
                case 1:
                    scalar *= QVector4D(static_cast<float>(cos(2.0 * PI * 1.0 / 8.0)), static_cast<float>(sin(2.0 * PI * 1.0 / 8.0)), static_cast<float>(cos(4.0 * PI * 1.0 / 8.0)), static_cast<float>(sin(4.0 * PI * 1.0 / 8.0)));
                    break;
                case 2:
                    scalar *= QVector4D(static_cast<float>(cos(2.0 * PI * 2.0 / 8.0)), static_cast<float>(sin(2.0 * PI * 2.0 / 8.0)), static_cast<float>(cos(4.0 * PI * 2.0 / 8.0)), static_cast<float>(sin(4.0 * PI * 2.0 / 8.0)));
                    break;
                case 3:
                    scalar *= QVector4D(static_cast<float>(cos(2.0 * PI * 3.0 / 8.0)), static_cast<float>(sin(2.0 * PI * 3.0 / 8.0)), static_cast<float>(cos(4.0 * PI * 3.0 / 8.0)), static_cast<float>(sin(4.0 * PI * 3.0 / 8.0)));
                    break;
                case 4:
                    scalar *= QVector4D(static_cast<float>(cos(2.0 * PI * 4.0 / 8.0)), static_cast<float>(sin(2.0 * PI * 4.0 / 8.0)), static_cast<float>(cos(4.0 * PI * 4.0 / 8.0)), static_cast<float>(sin(4.0 * PI * 4.0 / 8.0)));
                    break;
                case 5:
                    scalar *= QVector4D(static_cast<float>(cos(2.0 * PI * 5.0 / 8.0)), static_cast<float>(sin(2.0 * PI * 5.0 / 8.0)), static_cast<float>(cos(4.0 * PI * 5.0 / 8.0)), static_cast<float>(sin(4.0 * PI * 5.0 / 8.0)));
                    break;
                case 6:
                    scalar *= QVector4D(static_cast<float>(cos(2.0 * PI * 6.0 / 8.0)), static_cast<float>(sin(2.0 * PI * 6.0 / 8.0)), static_cast<float>(cos(4.0 * PI * 6.0 / 8.0)), static_cast<float>(sin(4.0 * PI * 6.0 / 8.0)));
                    break;
                case 7:
                    scalar *= QVector4D(static_cast<float>(cos(2.0 * PI * 7.0 / 8.0)), static_cast<float>(sin(2.0 * PI * 7.0 / 8.0)), static_cast<float>(cos(4.0 * PI * 7.0 / 8.0)), static_cast<float>(sin(4.0 * PI * 7.0 / 8.0)));
                    break;
            }

            // BIND OUR PROGRAM VERTEX AND INDICE BUFFERS FOR DRAWING THE FBO
            if (programA.bind()) {
                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // SET THE ACTIVE TEXTURE ON THE GPU
                        glActiveTexture(GL_TEXTURE0);
                        frameTexture->bind();
                        programA.setUniformValue("qt_texture", 0);

                        // SET THE SCALE FACTOR FOR 12 OUT OF 16 BITS PER PIXEL
                        programA.setUniformValue("qt_scaleFactor", scalar);

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

        // DISABLE ALPHA BLENDING
        glDisable(GL_BLEND);

        // SWAP THE INTERMEDIATE FFT BUFFER TO THE COMPLETED FFT BUFFER
        if (frm == 7) {
            QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectB, frameBufferObjectA);
        } else if (frm == 15) {
            QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectC, frameBufferObjectA);
        } else if (frm == 23) {
            QOpenGLFramebufferObject::blitFramebuffer(frameBufferObjectD, frameBufferObjectA);
        }

        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUBaslerGLFilter::updateBuffer()
{
    if (surface && makeCurrent(surface)) {
        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
        // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
        if (frameBufferObjectE && frameBufferObjectE->bind()) {
            // CLEAR THE FRAME BUFFER OBJECT FROM ANY PREVIOUS SCANS
            glClearColor(NAN, NAN, NAN, NAN);
            glViewport(0, 0, frameBufferObjectE->width(), frameBufferObjectE->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // BIND OUR PROGRAM VERTEX AND INDICE BUFFERS FOR DRAWING THE FBO
            if (programB.bind()) {
                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // SET THE ACTIVE TEXTURE ON THE GPU
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, frameBufferObjectB->texture());
                        programB.setUniformValue("qt_depthTextureLo", 0);

                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_2D, frameBufferObjectC->texture());
                        programB.setUniformValue("qt_depthTextureMe", 1);

                        glActiveTexture(GL_TEXTURE2);
                        glBindTexture(GL_TEXTURE_2D, frameBufferObjectD->texture());
                        programB.setUniformValue("qt_depthTextureHi", 2);

                        // SET THE SNR THRESHOLD FOR DELETING BAD POINTS
                        programB.setUniformValue("qt_snrThreshold", static_cast<float>(snrThreshold) / 1000.0f);
                        programB.setUniformValue("qt_mtnThreshold", static_cast<float>(qPow(static_cast<double>(mtnThreshold) / 1000.0, 4.0)));

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
            frameBufferObjectE->release();
        }

        // BIND THE FRAME BUFFER OBJECT FOR PROCESSING THE PHASE DFT COEFFICIENTS
        // ALONG WITH THE GLSL PROGRAMS THAT WILL DO THE PROCESSING
        if (frameBufferObjectF && frameBufferObjectF->bind()) {
            // CLEAR THE FRAME BUFFER OBJECT FROM ANY PREVIOUS SCANS
            glClearColor(NAN, NAN, NAN, NAN);
            glViewport(0, 0, frameBufferObjectF->width(), frameBufferObjectF->height());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // BIND OUR PROGRAM VERTEX AND INDICE BUFFERS FOR DRAWING THE FBO
            if (programC.bind()) {
                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // SET THE ACTIVE TEXTURE ON THE GPU
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, frameBufferObjectE->texture());
                        programC.setUniformValue("qt_depthTexture", 0);

                        if (textureMin) {
                            glActiveTexture(GL_TEXTURE1);
                            textureMin->bind();
                            programC.setUniformValue("qt_minTexture", 1);
                        }

                        if (textureMax) {
                            glActiveTexture(GL_TEXTURE2);
                            textureMax->bind();
                            programC.setUniformValue("qt_maxTexture", 2);
                        }

                        if (textureLookUpTable) {
                            glActiveTexture(GL_TEXTURE3);
                            textureLookUpTable->bind();
                            programC.setUniformValue("qt_lutTexture", 3);
                            programC.setUniformValue("qt_layers", (float)(textureLookUpTable->depth() - 1));
                        }

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
            frameBufferObjectF->release();
        }

        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUBaslerGLFilter::getFrame()
{
    static int counter = 0;
    if (surface && makeCurrent(surface)) {
        if (++counter == 20) {
            // BIND AND DOWNLOAD THE FINAL XYZG BUFFER AND SAVE TO DISK
            //glBindTexture(GL_TEXTURE_2D, frameBufferObjectE->texture());
            //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, reinterpret_cast<unsigned char *>(localObject.constPointer()));
            //localObject.save(QString("/tmp/fboE.tif"));
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAUBaslerGLFilter::scanObj()
{
    if (surface && makeCurrent(surface)) {
        LAUMemoryObject object(frameBufferObjectE->width(), frameBufferObjectE->height(), 4, sizeof(float));
        glBindTexture(GL_TEXTURE_2D, frameBufferObjectE->texture());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, reinterpret_cast<unsigned char *>(object.constPointer()));
        return (object);
    }
    return (LAUMemoryObject());
}
