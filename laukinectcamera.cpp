#include "laukinectcamera.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUKinectCamera::LAUKinectCamera(QObject *parent) : LAUAbstractCamera(parent), ptrKinectSensor(nullptr), tracker(nullptr), glFilter(nullptr)
{
    rgbRows = 0;
    rgbCols = 0;

    numRows = 0;
    numCols = 0;
    numFrms = 1;
    numBits = 12;

    // KEEP TRYING TO FIND CAMERAS WHILE LIBRARY SEARCHES NETWORK
    errorString = QString("No cameras found.");

    unsigned int numSensors = k4a_device_get_installed_count();
    if (numSensors > 0) {
        k4a_result_t hr = k4a_device_open(K4A_DEVICE_DEFAULT, &ptrKinectSensor);
        if (hr == K4A_RESULT_SUCCEEDED) {
            size_t serial_number_length = 0;
            k4a_buffer_result_t br = k4a_device_get_serialnum(ptrKinectSensor, nullptr, &serial_number_length);
            if (br == K4A_BUFFER_RESULT_TOO_SMALL) {
                QByteArray byteArray(serial_number_length, 0);
                br = k4a_device_get_serialnum(ptrKinectSensor, byteArray.data(), &serial_number_length);
                if (br == K4A_BUFFER_RESULT_SUCCEEDED) {
                    serialString = QString(byteArray);
                    makeString = QString("Microsoft");
                    modelString = QString("Azure Kinect");
                    qDebug() << serialString;
                }
            }

            k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
            config.camera_fps                 = K4A_FRAMES_PER_SECOND_15;
            config.depth_mode                 = K4A_DEPTH_MODE_NFOV_UNBINNED;
            config.color_format               = K4A_IMAGE_FORMAT_COLOR_BGRA32;
            config.color_resolution           = K4A_COLOR_RESOLUTION_720P;

            hr = k4a_device_start_cameras(ptrKinectSensor, &config);
            if (hr == K4A_RESULT_SUCCEEDED) {
                rgbCols = 1280;
                rgbRows = 720;

                numCols = 640;
                numRows = 576;
            } else {
                errorString = QString("Failed to start device.");
            }
            k4a_calibration_t sensor_calibration;
            hr = k4a_device_get_calibration(ptrKinectSensor, config.depth_mode, config.color_resolution, &sensor_calibration);
            if (hr == K4A_RESULT_SUCCEEDED) {
                k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
                tracker_config.processing_mode = K4ABT_TRACKER_PROCESSING_MODE_GPU; //Use K4ABT_TRACKER_PROCESSING_MODE_CPU for cpu only
                hr = k4abt_tracker_create(&sensor_calibration, tracker_config, &tracker);
                if (hr != K4A_RESULT_SUCCEEDED) {
                    errorString = QString("Body tracker initialization failed!");
                }
            }
            else {
              errorString = QString("Get depth camera calibration failed!");
            }
        } else {
            errorString = QString("Failed to open device.");
        }
    } else {
        errorString = QString("No devices found.");
    }

    errorString = QString();
    isConnected = true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUKinectCamera::~LAUKinectCamera()
{
    // CLOSE THE KINECT SENSOR AND SAFELY RELEASE ITS HANDLE
    if (ptrKinectSensor) {
        if (isConnected) {
            //Destroy Body Tracker
            k4abt_tracker_shutdown(tracker);
            k4abt_tracker_destroy(tracker);
            k4a_device_stop_cameras(ptrKinectSensor);
        }

        k4a_device_close(ptrKinectSensor);
    }

    // DELETE THE OPENGLCONTEXT AND ITS OFFSCREEN SURFACE
    if (glFilter) {
        delete glFilter;
    }

    if (scanBuffer.isValid()){
        scanBuffer.save(QString("C:/scanBuffer.tif"));
    }

    qDebug() << "LAUKinectCamera::~LAUKinectCamera()";
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKinectCamera::updateBuffer()
{
    static int frameCounter = 0;
    qDebug() << "LAUKinectCamera::updateBuffer()" << frameCounter;

    if (scanBuffer.isNull()) {
        scanBuffer = LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned short));
    }

    // MAKE SURE WE HAVE A VALID GLCONTEXT TO DO OUR IMAGE PROCESSING
    if (glFilter == nullptr) {
        glFilter = new LAUKinectGLFilter(numCols, numRows, rgbCols, rgbRows);
        glFilter->enablePortraitMode(portrait());
        glFilter->setLookUpTable(lookUpTable);
        glFilter->setShareContext(shareContext);
        glFilter->onSetSurface();
    } else if (glFilter->wasInitialized()) {
        k4a_capture_t ptrCapture = nullptr;
        k4a_image_t pDepthFrame = nullptr;
        k4a_image_t pColorFrame = nullptr;
        k4a_image_t pNerIRFrame = nullptr;
        k4abt_frame_t body_frame = nullptr;

        k4a_wait_result_t wr;
        int counter = 0;
        do {
            // GRAB BOTH THE LATEST COLOR AND DEPTH VIDEO FRAMES FROM THE SENSOR
            wr = k4a_device_get_capture(ptrKinectSensor, &ptrCapture, 1500);

            // NOW SEPARATE OUT THE DEPTH FRAME AND GET A POINTER TO ITS BUFFER
            if (wr == K4A_WAIT_RESULT_SUCCEEDED) {
                // SEE IF WE HAVE A VALID DEPTH OBJECT AND IF SO GRAB THE DEPTH IMAGE
                pDepthFrame = k4a_capture_get_depth_image(ptrCapture);
                if (pDepthFrame == nullptr) {
                    errorString = QString("Failed to retrieve depth image.");
                    emit emitError(errorString);
                    return;
                }

                // SEE IF WE HAVE A VALID COLOR OBJECT AND IF SO GRAB THE COLOR IMAGE
                pNerIRFrame = k4a_capture_get_ir_image(ptrCapture);
                if (pNerIRFrame == nullptr) {
                    errorString = QString("Failed to retrieve NIR image.");
                    emit emitError(errorString);
                    return;
                }

                // SEE IF WE HAVE A VALID COLOR OBJECT AND IF SO GRAB THE COLOR IMAGE
                pColorFrame = k4a_capture_get_color_image(ptrCapture);
                if (pColorFrame == nullptr) {
                    errorString = QString("Failed to retrieve color image.");
                    emit emitError(errorString);
                    return;
                }
            } else if (wr == K4A_WAIT_RESULT_FAILED) {
                errorString = QString("Failed to capture a frame of video.");
                emit emitError(errorString);
            }
        } while ((wr == K4A_WAIT_RESULT_TIMEOUT) && (++counter < 10000000));

        if (wr == K4A_WAIT_RESULT_SUCCEEDED) {
            // UPDATE THE FRAME COUNTER TO USER KNOWS WE GRABBED A VALID FRAME OF VIDEO
            frameCounter++;

            unsigned char *cBuffer = reinterpret_cast<unsigned char*>(k4a_image_get_buffer(pColorFrame));
            if (cBuffer) {
                glFilter->setVideo(cBuffer);
                emit emitVideFBO(glFilter->videoFBO());
            } else {
                errorString = QString("Failed to retrieve color frame buffer.");
                emit emitError(errorString);
            }

            unsigned short *dBuffer = reinterpret_cast<unsigned short*>(k4a_image_get_buffer(pDepthFrame));
            if (dBuffer) {
                memcpy(scanBuffer.constPointer(), dBuffer, scanBuffer.length());

                glFilter->setDepth(dBuffer);
                emit emitDeptFBO(glFilter->depthFBO());
                emit emitScanFBO(glFilter->scanFBO());
            } else {
                errorString = QString("Failed to retrieve depth frame buffer.");
                emit emitError(errorString);
            }
            qDebug() << dBuffer[40000];

            unsigned short *nBuffer = reinterpret_cast<unsigned short*>(k4a_image_get_buffer(pNerIRFrame));
            if (nBuffer) {
                glFilter->setFace(nBuffer);
                emit emitFaceFBO(glFilter->faceFBO());
            } else {
                errorString = QString("Failed to retrieve color frame buffer.");
                emit emitError(errorString);
            }

            k4a_wait_result_t queue_capture_result = k4abt_tracker_enqueue_capture(tracker, ptrCapture, 0);
            k4a_capture_release(ptrCapture); // Remember to release the sensor capture once you finish using it
            if (queue_capture_result == K4A_WAIT_RESULT_TIMEOUT)
            {
                // It should never hit timeout when K4A_WAIT_INFINITE is set.
                qDebug() << "Error! Add capture to tracker process queue timeout!\n";
            }
            else if (queue_capture_result == K4A_WAIT_RESULT_FAILED)
            {
                qDebug() << "Error! Add capture to tracker process queue failed!\n";
            }

            // Pop Result from Body Tracker
            k4a_wait_result_t pop_frame_result = k4abt_tracker_pop_result(tracker, &body_frame, 0);
            if (pop_frame_result == K4A_WAIT_RESULT_SUCCEEDED)
             {
                /************* Successfully get a body tracking result, process the result here ***************/
                //VisualizeResult
                //Release the bodyFrame
                size_t num_bodies = k4abt_frame_get_num_bodies(body_frame);
                if (num_bodies >  0){
                    qDebug() << "bodies are detected: " << num_bodies;
                    k4abt_frame_release(body_frame);
                    }
                else{
                    qDebug() << "No bodies detected!";
                    }
               }
            else if (pop_frame_result == K4A_WAIT_RESULT_TIMEOUT)
               {
                    //  It should never hit timeout when K4A_WAIT_INFINITE is set.
                    qDebug() << "Error! Pop body frame result timeout!\n";
                }
            else if (pop_frame_result == K4A_WAIT_RESULT_FAILED)
                {

                    qDebug() << "Pop body frame result failed!\n";
                }

            k4a_image_release(pDepthFrame);
            k4a_image_release(pColorFrame);
            k4a_image_release(pNerIRFrame);

        }
    }
}
