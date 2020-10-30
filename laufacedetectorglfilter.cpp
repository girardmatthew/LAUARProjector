#include "laufacedetectorglfilter.h"
#include "locale.h"

#ifdef USEOPENCV
using namespace std;
using namespace cv;
using namespace cv::face;
#endif

#ifdef USEVISAGE
using namespace VisageSDK;
using namespace cv::face;

int groupList[68] = {15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,4,14,4,14,4,4,14,4,14,4,14,14,14,14,9,9,9,9,9,3,12,12,3,12,12,3,12,12,3,12,12,8,8,8,8,8,8,8,8,8,8,8,8,2,2,2,2,2,2,2,2};
int indexListA[68] = {2,4,6,8,10,12,14,16,17,15,13,11,9,7,5,3,1,6,4,4,2,2,1,1,3,3,5,25,24,23,22,2,4,15,5,1,12,10,6,8,8,12,11,9,5,7,7,11,4,6,9,1,10,5,3,3,2,2,2,4,5,7,2,6,4,8,3,9};
int indexListB[68] = {2,4,6,8,10,12,14,16,17,15,13,11,9,7,5,3,1,6,4,4,2,2,1,1,3,3,5,25,24,23,22,2,4,15,5,1,12,10,6,8,8,12,11,9,5,7,7,11,4,6,9,1,10,5,3,7,7,2,8,8,5,7,2,6,4,8,3,9};
#endif

#define NUMBEROFFACIALFEATURES    68

QString LAUFaceDetectorGLFilter::faceDetectorString = QString("haarcascade_frontalface_alt2.xml");
QString LAUFaceDetectorGLFilter::faceMarkString = QString("lbfmodel.yaml");

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUFaceDetectorGLFilter::LAUFaceDetectorGLFilter(unsigned int cols, unsigned int rows, QObject *parent) : LAUAbstractGLFilter(cols, rows, parent), frameBufferObject(nullptr)
{
#if defined(USEVISAGE)
    inputImage = nullptr;

    QSettings settings;
    QString directory = settings.value("LAUFaceDetectorGLFilter::licenseKey", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString string = QFileDialog::getOpenFileName(nullptr, QString("Load license key..."), directory, QString("*.vlc"));
    if (string.isEmpty() == false) {
        settings.setValue("LAUFaceDetectorGLFilter::licenseKey", QFileInfo(string).absolutePath());

        string = QFileInfo(string).path();
        initializeLicenseManager(string.toLatin1());

        string.append("/data/Facial Features Tracker - Low.cfg");
        visageTracker = new VisageTracker(string.toUtf8());
    }
#elif defined(USEOPENCV)
    QSettings settings;
    QString directory = settings.value("LAUFacialFeatureDetectorGLWidget::faceDetectorModel", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    if (QDir(directory).exists() == false) {
        directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    faceDetectorString = QFileDialog::getOpenFileName(nullptr, QString("Load classifier..."), directory, QString("*.xml"));
    if (faceDetectorString.isEmpty() == false) {
        settings.setValue("LAUFacialFeatureDetectorGLWidget::faceDetectorModel", QFileInfo(faceDetectorString).absolutePath());

        faceDetector = new CascadeClassifier();
        if (faceDetector->load(faceDetectorString.toStdString())) {
            QString directory = settings.value("LAUFacialFeatureDetectorGLWidget::faceMarkModel", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
            if (QDir(directory).exists() == false) {
                directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            }
            faceMarkString = QFileDialog::getOpenFileName(nullptr, QString("Load classifier..."), directory, QString("*.yaml"));
            if (faceMarkString.isEmpty() == false) {
                settings.setValue("LAUFacialFeatureDetectorGLWidget::faceMarkModel", QFileInfo(faceMarkString).absolutePath());

                facemark = FacemarkLBF::create();
                facemark->loadModel(faceMarkString.toStdString());
            }
        }
    }
#endif
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUFaceDetectorGLFilter::~LAUFaceDetectorGLFilter()
{
    if (wasInitialized()) {
        if (surface && makeCurrent(surface)) {
            if (frameBufferObject) {
                delete frameBufferObject;
            }
        }
    }

#if defined(USEVISAGE)
    if (visageTracker){
        delete visageTracker;
    }
#elif defined(USEOPENCV)
    if (faceDetector) {
        faceDetector.release();
        delete faceDetector;
    }

    if (facemark) {
        facemark.release();
        delete facemark;
    }
#endif
    //objectA.save(QString("C:/objectAA.tif"));

    qDebug() << QString("LAUFaceDetectorGLFilter::~LAUFaceDetectorGLFilter()");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFaceDetectorGLFilter::initialize()
{
    // NOW ADD OUR LIST OF HARRIS CORNER SHADER PROGRAMS
    setlocale(LC_NUMERIC, "C");
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/displayRGBAVideo.vert");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/displayRGBAVideo.frag");
    program.link();
    setlocale(LC_ALL, "");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QOpenGLFramebufferObject *LAUFaceDetectorGLFilter::updateFBO(QOpenGLFramebufferObject *fbo)
{
    qDebug() << "LAUFaceDetectorGLFilter::updateFBO()";

    static int frameCounter = 0;

#if defined(USEVISAGE)
    // SEE IF WE NEED NEW FBOS
    if (fbo && surface && makeCurrent(surface)) {
        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // INITIALIZE THE FRAME BUFFER OBJECT BASED ON THE INCOMING TEXTURE SIZE
        if (frameBufferObject == nullptr) {
            // CREATE A FORMAT OBJECT FOR CREATING THE FRAME BUFFER
            QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
            frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);

            // CREATE A NEW FRAME BUFFER OBJECT
            frameBufferObject = new QOpenGLFramebufferObject(width(), height(), frameBufferObjectFormat);
            frameBufferObject->release();

            // CREATE A OPENCV MATRIX FOR HOLDING THE GRAYSCALE FRAME ON THE CPU
            grayFrame = Mat(frameBufferObject->height(), frameBufferObject->width(), CV_8U);

            //objectA = LAUMemoryObject(frameBufferObject->width(), frameBufferObject->height(), 4, sizeof(float));
        }

        // SET CLEAR COLOR AS NOT A NUMBERS
        glClearColor(NAN, NAN, NAN, NAN);

        // CONVERT THE RGB TEXTURE INTO GRAYSCALE
        if (faceDetector && frameBufferObject->bind()) {
            if (program.bind()) {
                // SET THE VIEWPOINT BUT DON'T CLEAR THE PREVIOUS CONTENTS OF THE BUFFER
                glViewport(0, 0, frameBufferObject->width(), frameBufferObject->height());
                glClearColor(NAN, NAN, NAN, NAN);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // SET THE ACTIVE TEXTURE ON THE GPU
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, fbo->texture());
                        program.setUniformValue("qt_texture", 0);
                        program.setUniformValue("qt_portrait", false);
                        program.setUniformValue("qt_flip", 1.0f);

                        // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                        program.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                        program.enableAttributeArray("qt_vertex");

                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                        quadIndexBuffer.release();
                    }
                    quadVertexBuffer.release();
                }
                program.release();
            }

            // RELEASE THE VERTEX ARRAY OBJECT
            frameBufferObject->release();

            // COPY FRAME BUFFER TEXTURE FROM GPU TO LOCAL CPU BUFFER
            glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, grayFrame.data);

            if (visageTracker){
                int *numFaces = visageTracker->track(grayFrame.cols, grayFrame.rows, (const char*)grayFrame.data, faceData, VISAGE_FRAMEGRABBER_FMT_LUMINANCE, VISAGE_FRAMEGRABBER_ORIGIN_TL, 0, -1, 1);
                if (frameCounter > 2 && numFaces[0] == TRACK_STAT_OK){
                    // CREATE A VECTOR TO HOLD THE LANDMARKS FOR EACH DETECTED FACE
                    QList<QPointF> points;

                    for (int n=0; n<68; n++){
                        FeaturePoint fpA = faceData[0].featurePoints2D->getFP(groupList[n],indexListA[n]);
                        FeaturePoint fpB = faceData[0].featurePoints2D->getFP(groupList[n],indexListB[n]);
                        float col = (fpA.pos[0] + fpB.pos[0]) / 2.0f * (float)grayFrame.cols;
                        float row = (fpA.pos[1] + fpB.pos[1]) / 2.0f * (float)grayFrame.rows;
                        points << QPointF(col, row);
                    }

                    // SEND THE FACE VERTICES TO THE USER
                    emit emitFaceVertices(points);
                }
            }
        }
        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();
    }
#elif defined(USEOPENCV)
    // SEE IF WE NEED NEW FBOS
    if (fbo && surface && makeCurrent(surface)) {
        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.bind();

        // INITIALIZE THE FRAME BUFFER OBJECT BASED ON THE INCOMING TEXTURE SIZE
        if (frameBufferObject == nullptr) {
            // CREATE A FORMAT OBJECT FOR CREATING THE FRAME BUFFER
            QOpenGLFramebufferObjectFormat frameBufferObjectFormat;
            frameBufferObjectFormat.setInternalTextureFormat(GL_RGBA32F);

            // CREATE A NEW FRAME BUFFER OBJECT
            frameBufferObject = new QOpenGLFramebufferObject(width(), height(), frameBufferObjectFormat);
            frameBufferObject->release();

            // CREATE A OPENCV MATRIX FOR HOLDING THE GRAYSCALE FRAME ON THE CPU
            grayFrame = Mat(frameBufferObject->height(), frameBufferObject->width(), CV_8U);

            //objectA = LAUMemoryObject(frameBufferObject->width(), frameBufferObject->height(), 4, sizeof(float));
        }

        // SET CLEAR COLOR AS NOT A NUMBERS
        glClearColor(NAN, NAN, NAN, NAN);

        // CONVERT THE RGB TEXTURE INTO GRAYSCALE
        if (faceDetector && frameBufferObject->bind()) {
            if (program.bind()) {
                // SET THE VIEWPOINT BUT DON'T CLEAR THE PREVIOUS CONTENTS OF THE BUFFER
                glViewport(0, 0, frameBufferObject->width(), frameBufferObject->height());
                glClearColor(NAN, NAN, NAN, NAN);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if (quadVertexBuffer.bind()) {
                    if (quadIndexBuffer.bind()) {
                        // SET THE ACTIVE TEXTURE ON THE GPU
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, fbo->texture());
                        program.setUniformValue("qt_texture", 0);
                        program.setUniformValue("qt_portrait", false);
                        program.setUniformValue("qt_flip", 1.0f);

                        // TELL OPENGL PROGRAMMABLE PIPELINE HOW TO LOCATE VERTEX POSITION DATA
                        program.setAttributeBuffer("qt_vertex", GL_FLOAT, 0, 4, 4 * sizeof(float));
                        program.enableAttributeArray("qt_vertex");

                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                        quadIndexBuffer.release();
                    }
                    quadVertexBuffer.release();
                }
                program.release();
            }

            // RELEASE THE VERTEX ARRAY OBJECT
            frameBufferObject->release();

            // COPY FRAME BUFFER TEXTURE FROM GPU TO LOCAL CPU BUFFER
            glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, grayFrame.data);
            //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, objectA.constPointer());

            // DOWNLOAD THE MAPPING BETWEEN CAMERA AND PROJECTOR
            //glBindTexture(GL_TEXTURE_2D, frameBufferObject->texture());
            //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, projectorObject.constPointer());
            //projectorObject.save(QString("/tmp/projectorObject.tif"));

            // ROTATE THE INPUT IMAGE CLOCKWISE
            rotate(grayFrame, grayFrame, cv::ROTATE_90_CLOCKWISE);

            // CREATE A VECTOR OF RECTANGLES TO HOLD ONE RECTANGLE PER FACE
            vector<Rect> faces;
            faceDetector->detectMultiScale(grayFrame, faces);

            // REMOVE ALL BUT THE LARGEST FACE
            if (faces.size() > 1) {
                int largestFaceIndex = 0;
                for (int n = 1; n < faces.size(); n++) {
                    if (faces.at(largestFaceIndex).area() < faces.at(n).area()) {
                        largestFaceIndex = n;
                    }
                }
                Rect face = faces.at(largestFaceIndex);
                faces.clear();
                faces.push_back(face);
            }

            // NEW SEE IF FOUND AT LEAST ONE FACE
            if (faces.size() > 0) {
                qDebug() << "Found a face!";

                // CREATE A VECTOR TO HOLD THE LANDMARKS FOR EACH DETECTED FACE
                vector< vector<Point2f> > landmarks;
                if (facemark->fit(grayFrame, faces, landmarks)) {
                    // COPY THE FACIAL FEATURE POINTS OVER TO OUR MEMORY BUFFER
                    QList<QPointF> points;
                    for (int n = 0; n < landmarks.at(0).size() && n < NUMBEROFFACIALFEATURES; n++) {
                        points << QPointF(landmarks.at(0).at(n).x, landmarks.at(0).at(n).y);
                    }
                    // SEND THE FACE VERTICES TO THE USER
                    emit emitFaceVertices(points);
                }
            }
        }
        // BIND THE VERTEX ARRAY OBJECT
        vertexArrayObject.release();
    }
#endif
    frameCounter++;

    return (fbo);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QList<QPointF> LAUFaceDetectorGLFilter::detectFacialFeatures(LAUMemoryObject object)
{
    QList<QPointF> points;

    Mat inputImage, grayFrame;
    if (object.colors() == 1) {
        if (object.depth() == sizeof(unsigned char)) {
            inputImage = Mat(object.height(), object.width(), CV_8U, object.constPointer(), object.step());
        } else if (object.depth() == sizeof(unsigned short)) {
            inputImage = Mat(object.height(), object.width(), CV_16U, object.constPointer(), object.step());
        } else if (object.depth() == sizeof(float)) {
            inputImage = Mat(object.height(), object.width(), CV_32F, object.constPointer(), object.step());
        }
        // CONVERT THE INCOMING IMAGE INTO GRAYSCALE
        grayFrame = inputImage;
    } else if (object.colors() == 3) {
        if (object.depth() == sizeof(unsigned char)) {
            inputImage = Mat(object.height(), object.width(), CV_8UC3, object.constPointer(), object.step());
        } else if (object.depth() == sizeof(unsigned short)) {
            inputImage = Mat(object.height(), object.width(), CV_16UC3, object.constPointer(), object.step());
        } else if (object.depth() == sizeof(float)) {
            inputImage = Mat(object.height(), object.width(), CV_32FC3, object.constPointer(), object.step());
        }
        // CONVERT THE INCOMING IMAGE INTO GRAYSCALE
        cvtColor(inputImage, grayFrame, COLOR_RGB2GRAY, 1);
    } else if (object.colors() == 4) {
        if (object.depth() == sizeof(unsigned char)) {
            inputImage = Mat(object.height(), object.width(), CV_8UC4, object.constPointer(), object.step());
        } else if (object.depth() == sizeof(unsigned short)) {
            inputImage = Mat(object.height(), object.width(), CV_16UC4, object.constPointer(), object.step());
        } else if (object.depth() == sizeof(float)) {
            inputImage = Mat(object.height(), object.width(), CV_32FC4, object.constPointer(), object.step());
        }
        // CONVERT THE INCOMING IMAGE INTO GRAYSCALE
        cvtColor(inputImage, grayFrame, COLOR_RGBA2GRAY, 1);
    }

    if (object.depth() == sizeof(float)){
        grayFrame = 255.0 * grayFrame;
    }
    grayFrame.convertTo(grayFrame, CV_8U);

#if defined(USEVISAGE)
    CascadeClassifier faceDetector;
    if (QFile::exists(faceDetectorString) && faceDetector.load(faceDetectorString.toStdString())) {
        // CREATE A VECTOR OF RECTANGLES TO HOLD ONE RECTANGLE PER FACE
        vector<Rect> faces;
        faceDetector.detectMultiScale(grayFrame, faces);

        // REMOVE ALL BUT THE LARGEST FACE
        if (faces.size() > 1) {
            int largestFaceIndex = 0;
            for (int n = 1; n < faces.size(); n++) {
                if (faces.at(largestFaceIndex).area() < faces.at(n).area()) {
                    largestFaceIndex = n;
                }
            }
            Rect face = faces.at(largestFaceIndex);
            faces.clear();
            faces.push_back(face);
        }

        // NEW SEE IF FOUND AT LEAST ONE FACE
        if (QFile::exists(faceMarkString) && faces.size() > 0) {
            Ptr<Facemark> facemark = FacemarkLBF::create();
            facemark->loadModel(faceMarkString.toStdString());

            // CREATE A VECTOR TO HOLD THE LANDMARKS FOR EACH DETECTED FACE
            vector< vector<Point2f> > landmarks;
            if (facemark->fit(grayFrame, faces, landmarks)) {
                // COPY THE FACIAL FEATURE POINTS OVER TO OUR MEMORY BUFFER
                for (int n = 0; n < landmarks.at(0).size() && n < NUMBEROFFACIALFEATURES; n++) {
                    points << QPointF(landmarks.at(0).at(n).x / (float)grayFrame.cols, landmarks.at(0).at(n).y / (float)grayFrame.rows);
                }
            }
        }
    }
#elif defined(USEOPENCV)
    CascadeClassifier faceDetector;
    if (QFile::exists(faceDetectorString) && faceDetector.load(faceDetectorString.toStdString())) {
        // CREATE A VECTOR OF RECTANGLES TO HOLD ONE RECTANGLE PER FACE
        vector<Rect> faces;
        faceDetector.detectMultiScale(grayFrame, faces);

        // REMOVE ALL BUT THE LARGEST FACE
        if (faces.size() > 1) {
            int largestFaceIndex = 0;
            for (int n = 1; n < faces.size(); n++) {
                if (faces.at(largestFaceIndex).area() < faces.at(n).area()) {
                    largestFaceIndex = n;
                }
            }
            Rect face = faces.at(largestFaceIndex);
            faces.clear();
            faces.push_back(face);
        }

        // NEW SEE IF FOUND AT LEAST ONE FACE
        if (QFile::exists(faceMarkString) && faces.size() > 0) {
            Ptr<Facemark> facemark = FacemarkLBF::create();
            facemark->loadModel(faceMarkString.toStdString());

            // CREATE A VECTOR TO HOLD THE LANDMARKS FOR EACH DETECTED FACE
            vector< vector<Point2f> > landmarks;
            if (facemark->fit(grayFrame, faces, landmarks)) {
                // COPY THE FACIAL FEATURE POINTS OVER TO OUR MEMORY BUFFER
                for (int n = 0; n < landmarks.at(0).size() && n < NUMBEROFFACIALFEATURES; n++) {
                    points << QPointF(landmarks.at(0).at(n).x / (float)grayFrame.cols, landmarks.at(0).at(n).y / (float)grayFrame.rows);
                }
            }
        }
    }
#endif
    return (points);
}
