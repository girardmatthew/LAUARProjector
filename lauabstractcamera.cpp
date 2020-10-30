#include "lauabstractcamera.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUAbstractCamera::LAUAbstractCamera(QObject *parent) : QObject(parent), numRows(ABSTRACTROWS), numCols(ABSTRACTCOLS), numFrms(24), bitsPerPixel(12), isConnected(false), camera(nullptr), shareContext(nullptr), sliFilter(nullptr)
{
    // KEEP TRYING TO FIND CAMERAS WHILE LIBRARY SEARCHES NETWORK
    errorString = QString("No cameras found.");

    // GET A LIST OF AVAILABLE CAMERAS
    QStringList availableCameralist = cameraList();
    if (numAvailableCameras) {
        // NOW SEE IF WE CAN CONNECT TO FIRST DETECTED CAMERA
        if (connectToHost(availableCameralist.first())) {
            errorString = QString();
            isConnected = true;
        } else {
            disconnectFromHost();
            isConnected = false;
        }
    }
    onUpdateExposure(8100);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUAbstractCamera::~LAUAbstractCamera()
{
    // DELETE THE OPENGLCONTEXT AND ITS OFFSCREEN SURFACE
    if (sliFilter) {
        delete sliFilter;
    }

    // DISCONNECT FROM CAMERA
    if (isConnected) {
        disconnectFromHost();
    }
    PylonTerminate();

    qDebug() << QString("LAUAbstractCamera::~LAUAbstractCamera()");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUAbstractCamera::reset()
{
    return (isConnected);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractCamera::updateBuffer(LAUMemoryObject object)
{
    // MAKE SURE WE HAVE A VALID GLCONTEXT TO DO OUR IMAGE PROCESSING
    if (sliFilter == nullptr) {
        sliFilter = new LAUSliGLFilter(numCols, numRows);
        sliFilter->setShareContext(shareContext);
        sliFilter->setLookUpTable(lookUpTable);
    }

    if (object.isValid()) {
        // RESET ERROR CODE
        object.setElapsed(0);

        // NOW SET LINE 2 OUTPUT TO HIGH TO RESET PROJECTOR
        camera->UserOutputValue.SetValue(true);

        // THE PARAMETER MAXNUMBUFFER CAN BE USED TO CONTROL THE COUNT OF BUFFERS
        // ALLOCATED FOR GRABBING. THE DEFAULT VALUE OF THIS PARAMETER IS 10.
        camera->MaxNumBuffer = numFrms;

        // KEEP TRACK OF HOW MANY FRAMES WE HAVE SO FAR COLLECTED
        unsigned int counter = 0;

        try {
            // START THE GRABBING OF NUMBTCS IMAGES.
            camera->StartGrabbing(numFrms);

            // GIVE THE CAMERA API TIME TO PREPARE FRAME GRABBING
            // QThread::msleep(100);

            // THIS SMART POINTER WILL RECEIVE THE GRAB RESULT DATA.
            CGrabResultPtr ptrGrabResult;

            while (camera->IsGrabbing() && counter < object.frames()) {
                // WAIT FOR AN IMAGE AND THEN RETRIEVE IT. A TIMEOUT OF 5000 MS IS USED.
                camera->RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

                // IMAGE GRABBED SUCCESSFULLY?
                if (ptrGrabResult->GrabSucceeded()) {
                    // SIMULTANEOUSLY DEBAYER THE INCOMING BUFFER AND COPY TO THE OBJECT BUFFER
                    debayer(object.constFrame(counter), reinterpret_cast<unsigned char *>(ptrGrabResult->GetBuffer()), numRows, numCols, numCols * sizeof(unsigned short));
                } else {
                    errorString = QString("Error grabbing frame:").append(QString(ptrGrabResult->GetErrorCode())).append(QString(ptrGrabResult->GetErrorDescription()));
                    object.setElapsed(static_cast<unsigned int>(255));
                }

                // SEND THE INCOMING FRAME TO THE GLCONTEXT FOR PROCESSING
                sliFilter->setFrame(reinterpret_cast<unsigned short *>(object.constFrame(counter)), static_cast<int>(counter));

                // INCREMENT THE COUNTER FOR THE NEXT FRAME
                counter++;
            }

            // NOW THAT WE HAVE ALL THE VIDEO, TELL THE GLCONTEXT TO FINISHING PROCESSING THE SCAN
            sliFilter->updateBuffer();
        } catch (const GenericException &e) {
            errorString = QString("Pylon exception:").append(QString(e.GetDescription()));
            emit emitError(errorString);
        }

        // NOW SET LINE 2 OUTPUT TO LOW TO RESET PROJECTOR
        camera->UserOutputValue.SetValue(false);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractCamera::onUpdateExposure(int microseconds)
{
    // SET THE CAMERA'S EXPOSURE
    if (camera && camera->IsOpen()) {
        camera->ExposureTime.SetValue(microseconds);
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
bool LAUAbstractCamera::connectToHost(QString hostString)
{
    // ONLY LOOK FOR CAMERAS SUPPORTED BY CAMERA_T.
    CDeviceInfo info;
    info.SetDeviceClass(CBaslerUsbInstantCamera::DeviceClass());

    // GET THE TRANSPORT LAYER FACTORY.
    CTlFactory &tlFactory = CTlFactory::GetInstance();

    // GET ALL ATTACHED DEVICES AND EXIT APPLICATION IF NO DEVICE IS FOUND.
    DeviceInfoList_t devices;
    numAvailableCameras = static_cast<unsigned int>(tlFactory.EnumerateDevices(devices));

    try {
        // PRINT A LIST OF THE CONNECTED CAMERAS
        for (unsigned int i = 0; i < numAvailableCameras; i++) {
            if (QString(devices[i].GetFriendlyName()) == hostString) {
                // CREATE AN INSTANT CAMERA OBJECT WITH THE FIRST FOUND CAMERA DEVICE MATCHING THE SPECIFIED DEVICE CLASS.
                camera = new CBaslerUsbInstantCamera(CTlFactory::GetInstance().CreateFirstDevice(info));

                // OPEN THE CAMERA FOR ACCESSING THE PARAMETERS.
                camera->Open();

                if (camera->IsOpen()) {
                    // GET THE MAKE, MODEL, AND SERIAL NUMBER STRINGS
                    makeString = QString(camera->DeviceVendorName.GetValue());
                    modelString = QString(camera->DeviceModelName.GetValue());
                    serialString = QString(camera->DeviceFirmwareVersion.GetValue());

                    // SEE IF THE CAMERA SENSOR IS A COLOR SENSOR
                    isColor = modelString.endsWith(QString("c"));

                    if (modelString.contains(QString("acA1920"))) {
                        // TUNE THE RED, GREEN, AND BLUE GAINS FOR MAGENTA PROJECTION
                        camera->Gain.SetValue(1.0);

                        // SET THE WIDTH AND LEFT EDGE OF ROI
                        if (IsWritable(camera->Width)) {
                            camera->Width.SetValue(1088);
                        }

                        if (IsWritable(camera->Height)) {
                            camera->Height.SetValue(960);
                        }

                        if (IsWritable(camera->CenterX)) {
                            camera->CenterX.SetValue(false);
                            camera->OffsetX.SetValue(448);
                        }

                        if (IsWritable(camera->CenterY)) {
                            camera->CenterY.SetValue(true);
                        }

                        // MAKE SURE WE HAVE THE CURRENT ROI SIZE IN MEMORY
                        numCols = static_cast<unsigned int>(camera->Width.GetValue());
                        numRows = static_cast<unsigned int>(camera->Height.GetValue());

                        if (isColor) {
                            // DISABLE COLOR CORRECTION
                            camera->LUTEnable.SetValue(false);
                            camera->LightSourcePreset.SetValue(LightSourcePreset_Off);
                            camera->ColorAdjustmentSelector.SetValue(ColorAdjustmentSelector_Green);
                            camera->ColorAdjustmentHue.SetValue(0.0);
                            camera->ColorAdjustmentSaturation.SetValue(1.0);

                            camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
                            camera->BalanceRatio.SetValue(1.0000);
                            camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Green);
                            camera->BalanceRatio.SetValue(1.4356);
                            camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Blue);
                            camera->BalanceRatio.SetValue(1.3645);

                            if (IsWritable(camera->PixelFormat)) {
                                camera->PixelFormat.SetValue(PixelFormat_BayerBG10);
                                bitsPerPixel = 10;
                            }
                        } else {
                            if (IsWritable(camera->PixelFormat)) {
                                camera->PixelFormat.SetValue(PixelFormat_Mono10);
                                bitsPerPixel = 10;
                            }
                        }
                    } else {
                        // SET THE REGION OF INTEREST TO THE FULL SENSOR
                        if (IsWritable(camera->OffsetX)) {
                            camera->OffsetX.SetValue(camera->OffsetX.GetMin());
                        }

                        if (IsWritable(camera->OffsetY)) {
                            camera->OffsetY.SetValue(camera->OffsetY.GetMin());
                        }

                        if (IsWritable(camera->Width)) {
                            camera->Width.SetValue(camera->Width.GetMax());
                            numCols = static_cast<unsigned int>(camera->Width.GetValue());
                        }

                        if (IsWritable(camera->Height)) {
                            camera->Height.SetValue(camera->Height.GetMax());
                            numRows = static_cast<unsigned int>(camera->Height.GetValue());
                        }

                        if (IsWritable(camera->PixelFormat)) {
                            camera->PixelFormat.SetValue(PixelFormat_Mono10);
                            bitsPerPixel = 10;
                        }
                    }

                    if (IsWritable(camera->ExposureAuto)) {
                        camera->ExposureAuto.SetValue(ExposureAuto_Off);
                    }

                    // THE PARAMETER MAXNUMBUFFER CAN BE USED TO CONTROL THE COUNT OF BUFFERS
                    // ALLOCATED FOR GRABBING. THE DEFAULT VALUE OF THIS PARAMETER IS 10.
                    camera->MaxNumBuffer = numFrms;

                    return (setSynchronization());
                }
                return (false);
            }
        }
    } catch (const GenericException &e) {
        errorString = QString("Pylon exception:").append(QString(e.GetDescription()));
        emit emitError(errorString);
    }

    // IF WE MAKE IT THIS FAR, BUT WE ARE NOT CONNECTED, THEN THE CAMERA STRING WASN'T FOUND
    return (false);
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUAbstractCamera::disconnectFromHost()
{
    if (camera && camera->IsOpen()) {
        camera->Close();
        if (camera->IsOpen() == false) {
            return (true);
        }
    }
    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUAbstractCamera::setSynchronization()
{
    if (camera && camera->IsOpen()) {
        // ENABLE THE CAMERA TO GET IT TO START RECORDING FRAMES OF VIDEO
        camera->TriggerSelector.SetValue(TriggerSelector_FrameStart);     // default: framestart
        camera->TriggerSource.SetValue(TriggerSource_Line1);              // default: line1
        camera->TriggerActivation.SetValue(TriggerActivation_RisingEdge); // default: rising edge
        camera->TriggerMode.SetValue(TriggerMode_On);                     // default: off

        camera->TriggerDelay.SetValue(0);                               // delay one frame of video

        // SET LINE 1 TO TRIGGER IN CHANNEL
        camera->LineSelector.SetValue(LineSelector_Line1);
        camera->LineMode.SetValue(LineMode_Input);

        // NOW SET LINE 2 OUTPUT TO LOW
        camera->UserOutputSelector.SetValue(UserOutputSelector_UserOutput1);
        camera->UserOutputValue.SetValue(false);

        // SET LINE 2 TO GENERAL PURPOSE OUTPUT
        camera->LineSelector.SetValue(LineSelector_Line2);
        camera->LineMode.SetValue(LineMode_Output);
        camera->LineSource.SetValue(LineSource_UserOutput1);
        camera->LineInverter.SetValue(true);

        // SET LINE 3 TO GENERAL PURPOSE INPUT
        camera->LineSelector.SetValue(LineSelector_Line3);
        camera->LineMode.SetValue(LineMode_Input);

        // SET LINE 4 TO FRAME TRIGGER READY
        camera->LineSelector.SetValue(LineSelector_Line4);
        camera->LineMode.SetValue(LineMode_Output);
        camera->LineSource.SetValue(LineSource_FrameTriggerWait);
        camera->LineInverter.SetValue(true);
        return (true);
    }
    return (false);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QStringList LAUAbstractCamera::cameraList()
{
    // ONLY LOOK FOR CAMERAS SUPPORTED BY CAMERA_T.
    CDeviceInfo info;
    info.SetDeviceClass(CBaslerUsbInstantCamera::DeviceClass());

    // GET THE TRANSPORT LAYER FACTORY.
    CTlFactory &tlFactory = CTlFactory::GetInstance();

    // GET ALL ATTACHED DEVICES AND EXIT APPLICATION IF NO DEVICE IS FOUND.
    DeviceInfoList_t devices;
    numAvailableCameras = static_cast<unsigned int>(tlFactory.EnumerateDevices(devices));

    // PRINT A LIST OF THE CONNECTED CAMERAS
    QStringList stringList;
    for (unsigned int i = 0; i < numAvailableCameras; i++) {
        stringList << QString(devices[i].GetFriendlyName());
    }
    return (stringList);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUAbstractCamera::debayer(unsigned char *otBuffer, unsigned char *inBuffer, unsigned int rows, unsigned int cols, unsigned int step)
{
    __m128i pixelVec;
    __m128i shuffleA = _mm_set_epi8(15, 14, 15, 14, 11, 10, 11, 10, 7, 6, 7, 6, 3, 2, 3, 2);
    __m128i shuffleB = _mm_set_epi8(13, 12, 13, 12, 9, 8, 9, 8, 5, 4, 5, 4, 1, 0, 1, 0);

    if (isColor) {
        for (unsigned int row = 0; row < rows; row++) {
            if (row % 2 == 0) {
                for (unsigned int col = 0; col < cols; col += 8) {
                    pixelVec = _mm_load_si128(reinterpret_cast<const __m128i *>(inBuffer + col * sizeof(unsigned short)));
                    pixelVec = _mm_shuffle_epi8(pixelVec, shuffleA);
                    _mm_store_si128(reinterpret_cast<__m128i *>(otBuffer + col * sizeof(unsigned short)), pixelVec);
                }
            } else {
                for (unsigned int col = 0; col < cols; col += 8) {
                    pixelVec = _mm_load_si128(reinterpret_cast<const __m128i *>(inBuffer + col * sizeof(unsigned short)));
                    pixelVec = _mm_shuffle_epi8(pixelVec, shuffleB);
                    _mm_store_si128(reinterpret_cast<__m128i *>(otBuffer + col * sizeof(unsigned short)), pixelVec);
                }
            }
            // INCREMENT THE BUFFER POINTERS TO THE NEXT ROW OF PIXELS
            inBuffer += step;
            otBuffer += step;
        }
    } else {
        // COPY OVER THE FRAME IN ONE GIANT BLOCK
        memcpy(otBuffer, inBuffer, step * rows);
    }
}
