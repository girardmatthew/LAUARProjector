#include "laubaslercamera.h"

using namespace Pylon;
using namespace Basler_UsbCameraParams;

bool LAUBaslerCamera::libraryInitializedFlag = false;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUBaslerCameraWidget::LAUBaslerCameraWidget(QWidget *parent, bool hasDepth) : QWidget(parent), exposure(0), snrThreshold(0), mtnThreshold(0), expSlider(nullptr), snrSlider(nullptr), mtnSlider(nullptr), expSpinBox(nullptr), snrSpinBox(nullptr), mtnSpinBox(nullptr)
{
    this->setWindowFlags(Qt::Tool);
    this->setWindowTitle(QString("Camera Settings"));
    this->setLayout(new QGridLayout());
    this->layout()->setContentsMargins(6, 6, 6, 6);
    this->layout()->setSpacing(3);
    this->setFixedSize(435, 115);

    QSettings settings;
    exposure     = settings.value(QString("LAUBaslerCameraWidget::exposure"), 5000).toInt();
    snrThreshold = settings.value(QString("LAUBaslerCameraWidget::snrThreshold"), 10).toInt();
    mtnThreshold = settings.value(QString("LAUBaslerCameraWidget::mtnThreshold"), 990).toInt();

    expSlider = new QSlider(Qt::Horizontal);
    expSlider->setMinimum(1);
    expSlider->setMaximum(20000);
    expSlider->setValue(exposure);

    expSpinBox = new QSpinBox();
    expSpinBox->setFixedWidth(80);
    expSpinBox->setAlignment(Qt::AlignRight);
    expSpinBox->setMinimum(1);
    expSpinBox->setMaximum(20000);
    expSpinBox->setValue(exposure);

    connect(expSlider, SIGNAL(valueChanged(int)), expSpinBox, SLOT(setValue(int)));
    connect(expSpinBox, SIGNAL(valueChanged(int)), expSlider, SLOT(setValue(int)));
    connect(expSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onUpdateExposurePrivate(int)));

    QLabel *label = new QLabel(QString("Exposure"));
    label->setToolTip(QString("exposure time of the camera in microseconds"));
    (reinterpret_cast<QGridLayout *>(this->layout()))->addWidget(label, 0, 0, 1, 1, Qt::AlignRight);
    (reinterpret_cast<QGridLayout *>(this->layout()))->addWidget(expSlider, 0, 1, 1, 1);
    (reinterpret_cast<QGridLayout *>(this->layout()))->addWidget(expSpinBox, 0, 2, 1, 1);

    if (hasDepth) {
        snrSlider = new QSlider(Qt::Horizontal);
        snrSlider->setMinimum(0);
        snrSlider->setMaximum(1000);
        snrSlider->setValue(snrThreshold);

        snrSpinBox = new QSpinBox();
        snrSpinBox->setFixedWidth(80);
        snrSpinBox->setAlignment(Qt::AlignRight);
        snrSpinBox->setMinimum(0);
        snrSpinBox->setMaximum(1000);
        snrSpinBox->setValue(snrThreshold);

        connect(snrSlider, SIGNAL(valueChanged(int)), snrSpinBox, SLOT(setValue(int)));
        connect(snrSpinBox, SIGNAL(valueChanged(int)), snrSlider, SLOT(setValue(int)));
        connect(snrSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onUpdateSnrThresholdPrivate(int)));

        label = new QLabel(QString("SNR Threshold"));
        label->setToolTip(QString("required streng of the scan signal"));
        (reinterpret_cast<QGridLayout *>(this->layout()))->addWidget(label, 1, 0, 1, 1, Qt::AlignRight);
        (reinterpret_cast<QGridLayout *>(this->layout()))->addWidget(snrSlider, 1, 1, 1, 1);
        (reinterpret_cast<QGridLayout *>(this->layout()))->addWidget(snrSpinBox, 1, 2, 1, 1);

        mtnSlider = new QSlider(Qt::Horizontal);
        mtnSlider->setMinimum(0);
        mtnSlider->setMaximum(1000);
        mtnSlider->setValue(mtnThreshold);

        mtnSpinBox = new QSpinBox();
        mtnSpinBox->setFixedWidth(80);
        mtnSpinBox->setAlignment(Qt::AlignRight);
        mtnSpinBox->setMinimum(0);
        mtnSpinBox->setMaximum(1000);
        mtnSpinBox->setValue(mtnThreshold);

        connect(mtnSlider, SIGNAL(valueChanged(int)), mtnSpinBox, SLOT(setValue(int)));
        connect(mtnSpinBox, SIGNAL(valueChanged(int)), mtnSlider, SLOT(setValue(int)));
        connect(mtnSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onUpdateMtnThresholdPrivate(int)));

        label = new QLabel(QString("MTN Threshold"));
        label->setToolTip(QString("amount of motion you can tolerate"));
        (reinterpret_cast<QGridLayout *>(this->layout()))->addWidget(label, 2, 0, 1, 1, Qt::AlignRight);
        (reinterpret_cast<QGridLayout *>(this->layout()))->addWidget(mtnSlider, 2, 1, 1, 1);
        (reinterpret_cast<QGridLayout *>(this->layout()))->addWidget(mtnSpinBox, 2, 2, 1, 1);

        triggerScanButton = new QPushButton(QString("Trigger Scan"));
        triggerScanButton->setFixedWidth(440 - 12);
        triggerScanButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(triggerScanButton, SIGNAL(pressed()), this, SLOT(onTriggerScanPrivate()));
        (reinterpret_cast<QGridLayout *>(this->layout()))->addWidget(triggerScanButton, 3, 0, 1, 4, Qt::AlignCenter);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QWidget *LAUBaslerCamera::widget(QWidget *parent)
{
    // CREATE A WIDGET FOR INTERFACING TO THE SCANNER
    LAUBaslerCameraWidget *widget = new LAUBaslerCameraWidget(parent);
    connect(widget, SIGNAL(emitUpdateExposure(int)), this, SLOT(onUpdateExposure(int)));
    connect(widget, SIGNAL(emitUpdateSNR(int)), this, SLOT(onUpdateSNR(int)));
    connect(widget, SIGNAL(emitUpdateMTN(int)), this, SLOT(onUpdateMTN(int)));
    connect(widget, SIGNAL(emitTriggerScan()), this, SLOT(onTriggerScan()));

    onUpdateExposure(widget->exp());
    onUpdateSNR(widget->snr());
    onUpdateMTN(widget->mtn());

    return (widget);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUBaslerCamera::LAUBaslerCamera(QObject *parent) : LAUAbstractCamera(parent), camera(nullptr), sliFilter(nullptr), mode(ModeScan)
{
    // SET SOME SUB-CLASS VARIABLES
    numFrms = 24;

    // INITIALIZE CAMERA LIBRARY AND UNLOAD IF ERROR
    if (!libraryInitializedFlag) {
        PylonInitialize();
        libraryInitializedFlag = true;
    }

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
    onUpdateExposure(7600);
    //onUpdateExposure(15800);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUBaslerCamera::~LAUBaslerCamera()
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

    qDebug() << "LAUBaslerCamera::~LAUBaslerCamera()";
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUBaslerCamera::reset()
{
    return (isConnected);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUBaslerCamera::onUpdateExposure(int microseconds)
{
    // SET THE CAMERA'S EXPOSURE
    if (camera && camera->IsOpen()) {
        if (microseconds > 1000 && microseconds < 32000) {
            //camera->ExposureTime.SetValue(microseconds);
        }
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
bool LAUBaslerCamera::connectToHost(QString hostString)
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

                    // REVERSE THE CAMERA AXES BECAUSE IT IS MOUNTED UPSIDE DOWN
                    if (IsWritable(camera->ReverseX)) {
                        camera->ReverseX.SetValue(true);
                        camera->ReverseY.SetValue(true);
                    }

                    // SEE IF THE CAMERA SENSOR IS A COLOR SENSOR
                    isColor = modelString.endsWith(QString("c"));

                    if (modelString.contains(QString("acA1920"))) {
                        // TUNE THE RED, GREEN, AND BLUE GAINS FOR MAGENTA PROJECTION
                        camera->Gain.SetValue(1.0);

                        // SET THE WIDTH AND LEFT EDGE OF ROI
                        if (IsWritable(camera->Width)) {
                            camera->Width.SetValue(672);
                        }

                        if (IsWritable(camera->Height)) {
                            camera->Height.SetValue(640);
                        }

                        if (IsWritable(camera->CenterX)) {
                            camera->CenterX.SetValue(true);
                        }

                        if (IsWritable(camera->CenterY)) {
                            camera->CenterY.SetValue(true);
                        }

                        // MAKE SURE WE HAVE THE CURRENT ROI SIZE IN MEMORY
                        numCols = static_cast<int>(camera->Width.GetValue());
                        numRows = static_cast<int>(camera->Height.GetValue());

                        if (isColor) {
                            // DISABLE COLOR CORRECTION
                            camera->LUTEnable.SetValue(false);
                            camera->LightSourcePreset.SetValue(LightSourcePreset_Off);
                            camera->ColorAdjustmentSelector.SetValue(ColorAdjustmentSelector_Green);
                            camera->ColorAdjustmentHue.SetValue(0.0);
                            camera->ColorAdjustmentSaturation.SetValue(1.0);

                            camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
                            camera->BalanceRatio.SetValue(1.2700);
                            camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Green);
                            camera->BalanceRatio.SetValue(1.0000);
                            camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Blue);
                            camera->BalanceRatio.SetValue(1.8000);

                            if (IsWritable(camera->PixelFormat)) {
                                camera->PixelFormat.SetValue(PixelFormat_BayerRG10);
                                numBits = 10;
                            }
                        } else {
                            if (IsWritable(camera->PixelFormat)) {
                                camera->PixelFormat.SetValue(PixelFormat_Mono10);
                                numBits = 10;
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
                            numCols = static_cast<int>(camera->Width.GetValue());
                        }

                        if (IsWritable(camera->Height)) {
                            camera->Height.SetValue(camera->Height.GetMax());
                            numRows = static_cast<int>(camera->Height.GetValue());
                        }

                        if (IsWritable(camera->PixelFormat)) {
                            camera->PixelFormat.SetValue(PixelFormat_Mono10);
                            numBits = 10;
                        }
                    }

                    if (IsWritable(camera->ExposureAuto)) {
                        camera->ExposureAuto.SetValue(ExposureAuto_Off);
                    }

                    // THE PARAMETER MAXNUMBUFFER CAN BE USED TO CONTROL THE COUNT OF BUFFERS
                    // ALLOCATED FOR GRABBING. THE DEFAULT VALUE OF THIS PARAMETER IS 10.
                    camera->MaxNumBuffer = numFrms;

                    // MAXIMIZE THE GAIN FACTOR
                    camera->GainSelector.SetValue(GainSelector_All);
                    camera->Gain.SetValue(1.0);
                    numBits = 10;

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
bool LAUBaslerCamera::disconnectFromHost()
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
bool LAUBaslerCamera::setSynchronization()
{
    if (camera && camera->IsOpen()) {
        // ENABLE THE CAMERA TO GET IT TO START RECORDING FRAMES OF VIDEO
        camera->TriggerSelector.SetValue(TriggerSelector_FrameStart);     // default: framestart
        camera->TriggerSource.SetValue(TriggerSource_Line1);              // default: line1
        camera->TriggerActivation.SetValue(TriggerActivation_RisingEdge); // default: rising edge
        camera->TriggerMode.SetValue(TriggerMode_On);                     // default: off

        camera->TriggerDelay.SetValue(500);                               // delay one frame of video

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
QStringList LAUBaslerCamera::cameraList()
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
void LAUBaslerCamera::updateBuffer()
{
    static int frameCounter = 0;
    qDebug() << "LAUBaslerCamera::updateBuffer()" << frameCounter++;

    if (frameBuffer.isNull()) {
        frameBuffer = LAUMemoryObject(numCols, numRows, 1, sizeof(unsigned short), numFrms);
    }

    // MAKE SURE WE HAVE A VALID GLCONTEXT TO DO OUR IMAGE PROCESSING
    if (sliFilter == nullptr) {
        sliFilter = new LAUBaslerGLFilter(numCols, numRows);
        sliFilter->onSetMTNThreshold(localMTN);
        sliFilter->onSetSNRThreshold(localSNR);
        sliFilter->setShareContext(shareContext);
        sliFilter->setLookUpTable(lookUpTable);
        sliFilter->onSetSurface();
    } else if (sliFilter->wasInitialized()) {
        if (mode == ModeScan) {
            // TOGGLE MODE FOR NEXT TIME AROUND
            mode = ModeFace;

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
                //QThread::msleep(100);

                // THIS SMART POINTER WILL RECEIVE THE GRAB RESULT DATA.
                CGrabResultPtr ptrGrabResult;

                while (camera->IsGrabbing() && counter < frameBuffer.frames()) {
                    // WAIT FOR AN IMAGE AND THEN RETRIEVE IT. A TIMEOUT OF 5000 MS IS USED.
                    camera->RetrieveResult(15000, ptrGrabResult, TimeoutHandling_ThrowException);

                    // IMAGE GRABBED SUCCESSFULLY?
                    if (ptrGrabResult->GrabSucceeded()) {
                        // SIMULTANEOUSLY DEBAYER THE INCOMING BUFFER AND COPY TO THE OBJECT BUFFER
                        debayer(frameBuffer.constFrame(counter), reinterpret_cast<unsigned char *>(ptrGrabResult->GetBuffer()), numRows, numCols, numCols * sizeof(unsigned short));
                    } else {
                        errorString = QString("Error grabbing frame:").append(QString(ptrGrabResult->GetErrorCode())).append(QString(ptrGrabResult->GetErrorDescription()));
                        frameBuffer.setElapsed(static_cast<unsigned int>(255));
                    }

                    // SEND THE INCOMING FRAME TO THE GLCONTEXT FOR PROCESSING
                    sliFilter->setFrame(reinterpret_cast<unsigned short *>(frameBuffer.constFrame(counter)), static_cast<int>(counter));

                    // INCREMENT THE COUNTER FOR THE NEXT FRAME
                    counter++;
                }

                // NOW THAT WE HAVE ALL THE VIDEO, TELL THE GLCONTEXT TO FINISHING PROCESSING THE SCAN
                sliFilter->updateBuffer();

                // DOWNLOAD THE SCAN FBO AS A MEMORY OBJECT AND EMIT TO USER
                emit emitScanObj(sliFilter->scanObj());
            } catch (const GenericException &e) {
                errorString = QString("Pylon exception:").append(QString(e.GetDescription()));
                emit emitError(errorString);
            }

            // NOW SET LINE 2 OUTPUT TO LOW TO RESET PROJECTOR
            camera->UserOutputValue.SetValue(false);
        } else if (mode == ModeFace) {
            // TOGGLE MODE FOR NEXT TIME AROUND
            mode = ModeFace;

            // NOW SET LINE 2 OUTPUT TO HIGH TO RESET PROJECTOR
            camera->UserOutputValue.SetValue(false);

            // THE PARAMETER MAXNUMBUFFER CAN BE USED TO CONTROL THE COUNT OF BUFFERS
            // ALLOCATED FOR GRABBING. THE DEFAULT VALUE OF THIS PARAMETER IS 10.
            camera->MaxNumBuffer = 1;

            // KEEP TRACK OF HOW MANY FRAMES WE HAVE SO FAR COLLECTED
            unsigned int counter = 0;

            try {
                // START THE GRABBING OF NUMBTCS IMAGES.
                camera->StartGrabbing(1);

                // THIS SMART POINTER WILL RECEIVE THE GRAB RESULT DATA.
                CGrabResultPtr ptrGrabResult;

                while (camera->IsGrabbing() && counter < 1) {
                    // WAIT FOR AN IMAGE AND THEN RETRIEVE IT. A TIMEOUT OF 5000 MS IS USED.
                    camera->RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

                    // IMAGE GRABBED SUCCESSFULLY?
                    if (ptrGrabResult->GrabSucceeded()) {
                        // COPY OVER THE FRAME IN ONE GIANT BLOCK
                        memcpy(frameBuffer.constFrame(counter), reinterpret_cast<unsigned char *>(ptrGrabResult->GetBuffer()), numRows * numCols * sizeof(short));
                    } else {
                        errorString = QString("Error grabbing frame:").append(QString(ptrGrabResult->GetErrorCode())).append(QString(ptrGrabResult->GetErrorDescription()));
                        frameBuffer.setElapsed(static_cast<unsigned int>(255));
                    }

                    // SEND THE INCOMING FRAME TO THE GLCONTEXT FOR PROCESSING
                    sliFilter->setFace(reinterpret_cast<unsigned short *>(frameBuffer.constFrame(counter)));

                    // INCREMENT THE COUNTER FOR THE NEXT FRAME
                    counter++;
                }

                // NOW THAT WE HAVE ALL THE VIDEO, TELL THE GLCONTEXT TO FINISHING PROCESSING THE SCAN
                sliFilter->flush();

                // EMIT THE SCAN FBO TO THE USER INTERFACE GLWIDGET, IF THERE IS A SHARE CONTEXT TO RECEIVE IT
                if (shareContext) {
                    qDebug() << "LAUBaslerCamera::emitFaceFBO()";
                    emit emitFaceFBO(sliFilter->faceFBO());
                }
            } catch (const GenericException &e) {
                errorString = QString("Pylon exception:").append(QString(e.GetDescription()));
                emit emitError(errorString);
            }
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUBaslerCamera::debayer(unsigned char *otBuffer, unsigned char *inBuffer, int rows, int cols, int step)
{
    if (isColor) {
        __m128i pixelVec;
        __m128i shuffleA = _mm_set_epi8(15, 14, 15, 14, 11, 10, 11, 10, 7, 6, 7, 6, 3, 2, 3, 2);
        __m128i shuffleB = _mm_set_epi8(13, 12, 13, 12, 9, 8, 9, 8, 5, 4, 5, 4, 1, 0, 1, 0);

        for (int row = 0; row < rows; row++) {
            if (row % 2 == 0) {
                for (int col = 0; col < cols; col += 8) {
                    pixelVec = _mm_load_si128(reinterpret_cast<const __m128i *>(inBuffer + col * sizeof(unsigned short)));
                    pixelVec = _mm_shuffle_epi8(pixelVec, shuffleA);
                    _mm_store_si128(reinterpret_cast<__m128i *>(otBuffer + col * sizeof(unsigned short)), pixelVec);
                }
            } else {
                for (int col = 0; col < cols; col += 8) {
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
        memcpy(otBuffer, inBuffer, static_cast<unsigned int>(step * rows));
    }
}

