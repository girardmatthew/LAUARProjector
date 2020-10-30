#include "laurealsensecamera.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURealSenseCamera::LAURealSenseCamera(QObject *parent) : LAUAbstractCamera(parent), camera(nullptr), glFilter(nullptr), context(nullptr)
{
    numRows = 480;
    numCols = 640;
    numFrms = 1;
    numBits = 8;

    // KEEP TRYING TO FIND CAMERAS WHILE LIBRARY SEARCHES NETWORK
    errorString = QString("No cameras found.");

    // CREATE A VARIABLE TO KEEP TRACK OF ERRORS
    rs2_error *err = nullptr;

    // CREATE A CONTEXT OBJECT. THIS OBJECT OWNS THE HANDLES TO ALL CONNECTED REALSENSE DEVICES.
    if (context == nullptr) {
        context = rs2_create_context(RS2_API_VERSION, &err);
        if (context == nullptr) {
            errorString = QString("Unable to initialize RealSense library!");
            return;
        }
    }

    // GET DEVICE COUNT
    cameraList = rs2_query_devices(context, &err);
    if (err) {
        errorString = QString("Unable to get device list: %1").arg(QString(rs2_get_error_message(err)));
        return;
    }

    int numCameras = rs2_get_device_count(cameraList, &err);
    if (err) {
        errorString = QString("Unable to get device count: %1").arg(QString(rs2_get_error_message(err)));
        return;
    } else if (numCameras == 0) {
        errorString = QString("No devices found!");
        return;
    }

    // OPEN DEVICE
    camera = rs2_create_device(cameraList, 0, &err);
    if (err) {
        errorString = QString("Unable to get device: %1").arg(QString(rs2_get_error_message(err)));
        return;
    }

    // GET CAMERA MAKE, MODEL, AND SERIAL STRING
    makeString = QString::fromLatin1(rs2_get_device_info(camera, RS2_CAMERA_INFO_NAME, &err));
    if (err) {
        errorString = QString("Unable to read make string: %1").arg(QString(rs2_get_error_message(err)));
        return;
    }

    serialString = QString::fromLatin1(rs2_get_device_info(camera, RS2_CAMERA_INFO_SERIAL_NUMBER, &err));
    if (err) {
        errorString = QString("Unable to read serial string: %1").arg(QString(rs2_get_error_message(err)));
        return;
    }

    // SET THE MODEL STRING
    modelString = QString("D415");

    // GET SENSOR COUNT
    rs2_sensor_list *sensorList = rs2_query_sensors(camera, &err);
    if (err) {
        errorString = QString("Unable to retreive valid context: %1").arg(QString(rs2_get_error_message(err)));
        emit emitError(errorString);
        return;
    }

    int numSensors = rs2_get_sensors_count(sensorList, &err);
    if (err) {
        errorString = QString("Unable to retreive valid context: %1").arg(QString(rs2_get_error_message(err)));
        emit emitError(errorString);
        return;
    } else if (numSensors == 0) {
        errorString = QString("Unable to retreive valid context: %1").arg(QString(rs2_get_error_message(err)));
        emit emitError(errorString);
        return;
    }

    // CREATE A LIST TO HOLD ALL THE DETECTED SENSORS
    QList<SensorPacket> sensors;

    // GET A LIST OF AVAILABLE STREAMS
    for (int n = 0; n < numSensors; n++) {
        SensorPacket sensorPacket;
        rs2_sensor *sensor = rs2_create_sensor(sensorList, n, &err);
        if (err) {
            errorString = QString("Unable to create sensor: %1").arg(QString(rs2_get_error_message(err)));
            emit emitError(errorString);
            return;
        } else if (sensor == nullptr) {
            errorString = QString("Unable to retreive valid sensor.");
            emit emitError(errorString);
            return;
        }

        // SET THE SENSOR STRING FOR THE CURRENT PACKET
        sensorPacket.sensorString = QString(rs2_get_sensor_info(sensor, RS2_CAMERA_INFO_NAME, &err));
        if (err) {
            errorString = QString("Unable to retreive sensor info: %1").arg(QString(rs2_get_error_message(err)));
            emit emitError(errorString);
            return;
        }

        // GET STREAM COUNT
        rs2_stream_profile_list *profiles = rs2_get_stream_profiles(sensor, &err);
        if (err) {
            errorString = QString("Unable to retreive stream profiles: %1").arg(QString(rs2_get_error_message(err)));
            emit emitError(errorString);
            return;
        }

        int numProfiles = rs2_get_stream_profiles_count(profiles, &err);
        if (err) {
            errorString = QString("Unable to retreive stream profiles count: %1").arg(QString(rs2_get_error_message(err)));
            emit emitError(errorString);
            return;
        } else if (numProfiles == 0) {
            errorString = QString("No available profiles.");
            emit emitError(errorString);
            return;
        }

        for (int n = 0; n < numProfiles; n++) {
            StreamPacket streamPacket;

            streamPacket.profile = rs2_get_stream_profile(profiles, n, &err);
            if (err) {
                errorString = QString("Unable to retreive stream profile: %1").arg(QString(rs2_get_error_message(err)));
                emit emitError(errorString);
                return;
            } else if (streamPacket.profile == nullptr) {
                errorString = QString("Unable to retreive valid profile.");
                emit emitError(errorString);
                return;
            }

            rs2_get_stream_profile_data(streamPacket.profile, &streamPacket.stream, &streamPacket.format, &streamPacket.index, &streamPacket.uniqueID, &streamPacket.frameRate, &err);
            if (err) {
                errorString = QString("Unable to retreive stream profile data: %1").arg(QString(rs2_get_error_message(err)));
                emit emitError(errorString);
                return;
            }
            sensorPacket.streams << streamPacket;
        }
        sensors << sensorPacket;

        // WE NEED TO DELETE THE SENSOR NOW THAT WE ARE DONE WITH IT
        rs2_delete_sensor(sensor);
    }

    // CREATE A PIPELINE TO CONFIGURE, START AND STOP CAMERA STREAMING
    pipeline = rs2_create_pipeline(context, &err);
    if (err) {
        errorString = QString("Unable to create pipeline: %1").arg(QString(rs2_get_error_message(err)));
        emit emitError(errorString);
        return;
    }

    // CREATE A CONFIG INSTANCE, USED TO SPECIFY HARDWARE CONFIGURATION
    configure = rs2_create_config(&err);
    if (err) {
        errorString = QString("Unable to configure pipeline: %1").arg(QString(rs2_get_error_message(err)));
        emit emitError(errorString);
        return;
    }

    // FIND THE PACKET CORRESPONDING TO THE INFRARED VIDEO STREAM
    StreamPacket packetA = findStream(sensors.at(0), RS2_STREAM_INFRARED, RS2_FORMAT_Y8, numCols, numRows, 60);
    if (packetA.profile == nullptr) {
        errorString = QString("Unable to find infrared stream.");
        emit emitError(errorString);
        return;
    }

    // GET THE INTRINSIC COLOR CAMERA PARAMETERS
    //rs2_get_video_stream_intrinsics(packetA.profile, &intrinsicsClr, &err);
    //if (err) {
    //    errorString = QString("Unable to read infrared sensor intrinsics : %1").arg(QString(rs2_get_error_message(err)));
    //    return;
    //}

    // ENABLE INFRARED VIDEO STREAM
    rs2_config_enable_stream(configure, packetA.stream, packetA.index, numCols, numRows, packetA.format, packetA.frameRate, &err);
    if (err) {
        errorString = QString("Unable to configure color stream: %1").arg(QString(rs2_get_error_message(err)));
        emit emitError(errorString);
        return;
    }

    // FIND THE PACKET CORRESPONDING TO THE DEPTH VIDEO STREAM
    StreamPacket packetB = findStream(sensors.at(0), RS2_STREAM_DEPTH, RS2_FORMAT_Z16, numCols, numRows, 60);
    if (packetB.profile == nullptr) {
        errorString = QString("Unable to find depth stream.");
        emit emitError(errorString);
        return;
    }

    // IF WE MADE IT THIS FAR, THEN THE DEPTH SENSOR IS THE FIRST IN THE LIST
    //rs2_sensor *sensor = rs2_create_sensor(sensorList, 0, &err);
    //if (err) {
    //    errorString = QString("Unable to create depth sensor: %1").arg(QString(rs2_get_error_message(err)));
    //    emit emitError(errorString);
    //    return;
    //} else if (sensor == nullptr) {
    //    errorString = QString("No valid depth sensor.");
    //    emit emitError(errorString);
    //    return;
    //}

    // DELETE THE SENSOR NOW THAT WE ARE DONE WITH IT
    //rs2_delete_sensor(sensor);

    // ENABLE DEPTH VIDEO STREAM
    rs2_config_enable_stream(configure, packetB.stream, packetB.index, numCols, numRows, packetB.format, packetB.frameRate, &err);
    if (err) {
        errorString = QString("Unable to configure depth stream: %1").arg(QString(rs2_get_error_message(err)));
        emit emitError(errorString);
        return;
    }

    // FIND THE PACKET CORRESPONDING TO THE RGB VIDEO STREAM
    StreamPacket packetC = findStream(sensors.at(1), RS2_STREAM_COLOR, RS2_FORMAT_RGB8, numCols, numRows, 60);
    if (packetC.profile == nullptr) {
        errorString = QString("Unable to find RGB stream.");
        emit emitError(errorString);
        return;
    }

    // ENABLE DEPTH VIDEO STREAM
    rs2_config_enable_stream(configure, packetC.stream, packetC.index, numCols, numRows, packetC.format, packetC.frameRate, &err);
    if (err) {
        errorString = QString("Unable to configure RGB stream: %1").arg(QString(rs2_get_error_message(err)));
        emit emitError(errorString);
        return;
    }

    // CLEAR OUR SENSORS LIST
    sensors.clear();

    // DELETE THE SENSOR LIST NOW THAT WE ARE DONE WITH IT
    rs2_delete_sensor_list(sensorList);

    // START THE PIPELINE STREAMING
    profile = rs2_pipeline_start_with_config(pipeline, configure, &err);
    if (err) {
        errorString = QString("Unable to start pipeline: %1").arg(QString(rs2_get_error_message(err)));
        emit emitError(errorString);
        return;
    }

    errorString = QString();
    isConnected = true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURealSenseCamera::~LAURealSenseCamera()
{
    rs2_error *err = nullptr;

    if (pipeline) {
        // STOP THE PIPELINE STREAMING
        rs2_pipeline_stop(pipeline, &err);
        if (err) {
            errorString = QString("Unable to stop the pipeline streaming: %1").arg(QString(rs2_get_error_message(err)));
        }
    }

    // DELETE PIPELINE PROFILE OBJECT
    if (profile) {
        rs2_delete_pipeline_profile(profile);
    }

    // DELETE THE PIPELINE CONFIGURATION OBJECT
    if (configure) {
        rs2_delete_config(configure);
    }

    // DELETE PIPELINE OBJECT
    if (pipeline) {
        rs2_delete_pipeline(pipeline);
    }

    // DELETE THE CAMERA OBJECT
    if (camera) {
        rs2_delete_device(camera);
    }

    // DELETE THE CAMERA LIST
    if (cameraList) {
        rs2_delete_device_list(cameraList);
    }

    // DELETE THE CONTEXT OBJECT
    if (context) {
        rs2_delete_context(context);
    }

    // DELETE THE OPENGLCONTEXT AND ITS OFFSCREEN SURFACE
    if (glFilter) {
        delete glFilter;
    }

    qDebug() << "LAURealSenseCamera::~LAURealSenseCamera()";
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURealSenseCamera::updateBuffer()
{
    static int frameCounter = 0;
    qDebug() << "LAURealSenseCamera::updateBuffer()" << frameCounter;

    if (scanBuffer.isNull()) {
        scanBuffer = LAUMemoryObject(numCols, numRows, 4, sizeof(float));
    }

    // MAKE SURE WE HAVE A VALID GLCONTEXT TO DO OUR IMAGE PROCESSING
    if (glFilter == nullptr) {
        glFilter = new LAURealSenseGLFilter(numCols, numRows);
        glFilter->enablePortraitMode(portrait());
        glFilter->setLookUpTable(lookUpTable);
        glFilter->setShareContext(shareContext);
        glFilter->onSetSurface();
    } else if (glFilter->wasInitialized()) {
        // CREATE A VARIABLE TO KEEP TRACK OF ERRORS
        rs2_error *err = nullptr;

        // WAIT FOR A NEW FRAME OF VIDEO FROM THE PIPELINE
        rs2_frame *frames = rs2_pipeline_wait_for_frames(pipeline, 2000, &err);
        if (err) {
            errorString = QString("Failed waiting for new frames of video: %1").arg(QString(rs2_get_error_message(err)));
            emit emitError(errorString);
            rs2_release_frame(frames);
        } else {
            // KEEP TRACK OF THE NUMBER OF CAPTURED FRAMES
            frameCounter++;

            // RETURNS THE NUMBER OF FRAMES EMBEDDED WITHIN THE COMPOSITE FRAME
            int numFrames = rs2_embedded_frames_count(frames, &err);
            if (err) {
                errorString = QString("Failed receiving one frame of video A: %1").arg(QString(rs2_get_error_message(err)));
                emit emitError(errorString);
            } else if (numFrames < 1) {
                errorString = QString("Failed receiving one frame of video B: %1").arg(QString(rs2_get_error_message(err)));
                emit emitError(errorString);
            } else {
                for (int n = 0; n < numFrames; n++) {
                    // GRAB THE HANDLE TO THE NEXT FRAME IN THE QUE
                    rs2_frame *frame = rs2_extract_frame(frames, n, &err);
                    if (err) {
                        errorString = QString("Failed grabbing frame handle: %1").arg(QString(rs2_get_error_message(err)));
                        emit emitError(errorString);
                    } else {
                        // CHECK IF THE GIVEN FRAME IS A DEPTH FRAME
                        if (rs2_is_frame_extendable_to(frame, RS2_EXTENSION_DEPTH_FRAME, &err)) {
                            const unsigned short *buffer = reinterpret_cast<const unsigned short *>(rs2_get_frame_data(frame, &err));
                            if (err) {
                                errorString = QString("Failed grabbing buffer to depth data: %1").arg(QString(rs2_get_error_message(err)));
                                emit emitError(errorString);
                            } else {
                                glFilter->setDepth(buffer);
                                emit emitDeptFBO(glFilter->depthFBO());
                                emit emitScanFBO(glFilter->scanFBO());
                            }
                        } else if (rs2_get_frame_bits_per_pixel(frame, &err) == 24) {
                            const unsigned char *buffer = reinterpret_cast<const unsigned char *>(rs2_get_frame_data(frame, &err));
                            if (err) {
                                errorString = QString("Failed grabbing buffer to rgb data: %1").arg(QString(rs2_get_error_message(err)));
                                emit emitError(errorString);
                            } else {
                                glFilter->setVideo(buffer);
                                emit emitVideFBO(glFilter->videoFBO());
                            }
                        } else {
                            const unsigned char *buffer = reinterpret_cast<const unsigned char *>(rs2_get_frame_data(frame, &err));
                            if (err) {
                                errorString = QString("Failed grabbing buffer to infrared data: %1").arg(QString(rs2_get_error_message(err)));
                                emit emitError(errorString);
                            } else {
                                glFilter->setFace(buffer);
                                emit emitFaceFBO(glFilter->faceFBO());
                            }
                        }
                    }
                    // RELEASE THE FRAME HANDLE
                    rs2_release_frame(frame);
                }
            }
        }
        // RELEASE THE HANDLE THE GRABBED FRAMES
        rs2_release_frame(frames);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURealSenseCamera::StreamPacket LAURealSenseCamera::findStream(SensorPacket sensor, rs2_stream stream, rs2_format format, int wdth, int hght, int fps)
{
    LAURealSenseCamera::StreamPacket packet = {-1, -1, -1, RS2_STREAM_ANY, RS2_FORMAT_ANY, nullptr };

    // GET THE STREAMS FOR THE REQUESTED SENSOR
    QList<LAURealSenseCamera::StreamPacket> packets = sensor.streams;

    // REMOVE ANY PACKETS THAT DON'T HAVE THE RIGHT FORMAT OR STREAM
    for (int n = packets.count() - 1; n >= 0; n--) {
        if (packets.at(n).stream != stream) {
            packets.removeAt(n);
            continue;
        } else if (packets.at(n).format != format) {
            packets.removeAt(n);
            continue;
        } else if (packets.at(n).frameRate != fps) {
            packets.removeAt(n);
            continue;
        }

        // CREATE A VARIABLE TO KEEP TRACK OF ERRORS
        rs2_error *err = nullptr;

        // GET THE INTRINSIC COLOR CAMERA PARAMETERS
        rs2_intrinsics intrinsics;
        rs2_get_video_stream_intrinsics(packets.at(n).profile, &intrinsics, &err);
        if (err) {
            errorString = QString("Unable to read infrared sensor intrinsics : %1").arg(QString(rs2_get_error_message(err)));
            return (packet);
        } else if (intrinsics.width != wdth) {
            packets.removeAt(n);
            continue;
        } else if (intrinsics.height != hght) {
            packets.removeAt(n);
            continue;
        }
    }

    // RETURN THE TOP RANKED PACKET
    if (packets.isEmpty()) {
        return (packet);
    } else {
        // FIND THE PACKET TO THE LOWEST UNIQUE ID
        packet = packets.first();
        for (int n = 0; n < packets.count(); n++) {
            if (packets.at(n).uniqueID < packet.uniqueID) {
                packet = packets.at(n);
            }
        }
        return (packet);
    }
}
