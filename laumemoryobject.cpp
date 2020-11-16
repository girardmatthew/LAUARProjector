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
#include "laumemoryobject.h"

#ifdef Q_PROCESSOR_ARM
void *_mm_malloc(int size, int align)
{
    Q_UNUSED(align);
    return (malloc(size));
}

void _mm_free(void *pointer)
{
    free(pointer);
}
#endif

using namespace libtiff;

int LAUMemoryObjectData::instanceCounter = 0;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectData::LAUMemoryObjectData() : QSharedData()
{
    numRows = 0;
    numCols = 0;
    numChns = 0;
    numByts = 0;
    numFrms = 0;

    stepBytes = 0;
    frameBytes = 0;

    buffer = nullptr;
    numBytesTotal = 0;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectData::LAUMemoryObjectData(unsigned int cols, unsigned int rows, unsigned int chns, unsigned int byts, unsigned int frms) : LAUMemoryObjectData()
{
    numRows = rows;
    numCols = cols;
    numChns = chns;
    numByts = byts;
    numFrms = frms;

    buffer = nullptr;
    stepBytes = 0;
    frameBytes = 0;
    numBytesTotal = 0;

    allocateBuffer();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectData::LAUMemoryObjectData(unsigned long long bytes) : LAUMemoryObjectData()
{
    numRows = bytes;
    numCols = 1;
    numChns = 1;
    numByts = 1;
    numFrms = 1;

    buffer = nullptr;
    stepBytes = 0;
    frameBytes = 0;
    numBytesTotal = 0;

    allocateBuffer();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectData::LAUMemoryObjectData(const LAUMemoryObjectData &other) : LAUMemoryObjectData()
{
    // COPY OVER THE SIZE PARAMETERS FROM THE SUPPLIED OBJECT
    numRows = other.numRows;
    numCols = other.numCols;
    numChns = other.numChns;
    numByts = other.numByts;
    numFrms = other.numFrms;

    // SET THESE VARIABLES TO ZERO AND LET THEM BE MODIFIED IN THE ALLOCATION METHOD
    buffer = nullptr;
    stepBytes = 0;
    frameBytes = 0;
    numBytesTotal = 0;

    allocateBuffer();
    if (buffer) {
        memcpy(buffer, other.buffer, numBytesTotal);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectData::~LAUMemoryObjectData()
{
    if (buffer != nullptr) {
        instanceCounter = instanceCounter - 1;
        qDebug() << QString("LAUMemoryObjectData::~LAUMemoryObjectData() %1").arg(instanceCounter) << numRows << numCols << numChns << numByts << numFrms << numBytesTotal;
        _mm_free(buffer);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMemoryObjectData::allocateBuffer()
{
    // ALLOCATE SPACE FOR HOLDING PIXEL DATA BASED ON NUMBER OF CHANNELS AND BYTES PER PIXEL
    numBytesTotal  = static_cast<unsigned long long>(numRows);
    numBytesTotal *= static_cast<unsigned long long>(numCols);
    numBytesTotal *= static_cast<unsigned long long>(numChns);
    numBytesTotal *= static_cast<unsigned long long>(numByts);
    numBytesTotal *= static_cast<unsigned long long>(numFrms);

    if (numBytesTotal > 0) {
        instanceCounter = instanceCounter + 1;
        qDebug() << QString("LAUMemoryObjectData::allocateBuffer() %1").arg(instanceCounter) << numRows << numCols << numChns << numByts << numFrms;

        stepBytes  = numCols * numChns * numByts;
        frameBytes = numCols * numChns * numByts * numRows;
        buffer     = _mm_malloc(numBytesTotal + 128, 16);
        if (buffer == nullptr) {
            qDebug() << QString("LAUVideoBufferData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
            qDebug() << QString("LAUVideoBufferData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
            qDebug() << QString("LAUVideoBufferData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
        }
    } else {
        qDebug() << QString("LAUMemoryObjectData::allocateBuffer() %1").arg(instanceCounter) << numRows << numCols << numChns << numByts << numFrms;
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject::LAUMemoryObject(QString filename) : data(new LAUMemoryObjectData()), transformMatrix(QMatrix4x4()), anchorPt(QPoint(-1, -1)), elapsedTime(0)
{
    // GET A FILE TO OPEN FROM THE USER IF NOT ALREADY PROVIDED ONE
    if (filename.isNull()) {
#ifndef HEADLESS
        QSettings settings;
        QString directory = settings.value("LAUMemoryObject::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        filename = QFileDialog::getOpenFileName(nullptr, QString("Load scan from disk (*.tif)"), directory); //, QString("*.tif;*.tiff"));
        if (filename.isEmpty() == false) {
            settings.setValue("LAUMemoryObject::lastUsedDirectory", QFileInfo(filename).absolutePath());
        } else {
            return;
        }
#else
        return;
#endif
    }

    // IF WE HAVE A VALID TIFF FILE, LOAD FROM DISK
    // OTHERWISE TRY TO CONNECT TO SCANNER
    if (QFile::exists(filename)) {
        // OPEN INPUT TIFF FILE FROM DISK
        TIFF *inTiff = TIFFOpen(filename.toLatin1(), "r");
        if (!inTiff) {
            return;
        }
        load(inTiff);
        TIFFClose(inTiff);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject::LAUMemoryObject(QImage image) : data(new LAUMemoryObjectData()), transformMatrix(QMatrix4x4()), anchorPt(QPoint(-1, -1)), elapsedTime(0)
{
    if (image.isNull() == false) {
        data->numCols = (unsigned int)image.width();
        data->numRows = (unsigned int)image.height();
        data->numByts = sizeof(float);
        data->numChns = 4;
        data->numFrms = 1;
        data->allocateBuffer();

        for (unsigned int row = 0; row < this->height(); row++) {
            float *toBuffer = (float *)constScanLine(row);
            for (unsigned int col = 0; col < this->width(); col++) {
                QRgb pixel = image.pixel(col, row);
                toBuffer[4 * col + 0] = (float)qRed(pixel) / 255.0f;
                toBuffer[4 * col + 1] = (float)qGreen(pixel) / 255.0f;
                toBuffer[4 * col + 2] = (float)qBlue(pixel) / 255.0f;
                toBuffer[4 * col + 3] = 1.0f;
            }
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject::LAUMemoryObject(TIFF *inTiff, int index) : data(new LAUMemoryObjectData()), transformMatrix(QMatrix4x4()), anchorPt(QPoint(-1, -1)), elapsedTime(0)
{
    load(inTiff, static_cast<unsigned short>(index));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUMemoryObject::save(QString filename, QWidget *parent)
{
    // LET THE USER SELECT A FILE FROM THE FILE DIALOG
    if (filename.isNull()) {
#ifndef HEADLESS
        QSettings settings;
        QString directory = settings.value("LAUMemoryObject::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        if (QDir().exists(directory) == false) {
            directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        }
        filename = QFileDialog::getSaveFileName(parent, QString("Save image to disk (*.tif)"), directory);
        if (filename.isEmpty() == false) {
            if (filename.toLower().endsWith(".tif") == false && filename.toLower().endsWith(".tiff") == false) {
                filename.append(".tif");
            }
            settings.setValue("LAUMemoryObject::lastUsedDirectory", QFileInfo(filename).absolutePath());
        } else {
            return (false);
        }
#else
        return (false);
#endif
    }

    // OPEN TIFF FILE FOR SAVING THE IMAGE USING BIGTIFF FOR LARGE FILES
    TIFF *outputTiff = (this->length() > 100000000) ? TIFFOpen(filename.toLatin1(), "w8") : TIFFOpen(filename.toLatin1(), "w");
    if (!outputTiff) {
        return (false);
    }

    // CREATE A RETURN FLAG
    bool flag = true;

    if (frames() == 1) {
        // WRITE IMAGE TO CURRENT DIRECTORY AND SAVE RESULT TO BOOL FLAG
        flag = save(outputTiff);
    } else {
        // CREATE A MEMORY OBJECT TO HOLD A SINGLE FRAME OF VIDEO
        LAUMemoryObject object(width(), height(), colors(), depth(), 1);
        for (unsigned int n = 0; n < frames(); n++) {
            // COPY THE CURRENT FRAME INTO THE TEMPORARY FRAME BUFFER OBJECT
            memcpy(object.constPointer(), frame(n), block());

            // SAVE THE CURRENT FRAME INTO ITS OWN DIRECTORY INSIDE THE NEW TIFF FILE
            if (object.save(outputTiff, n) == false) {
                flag = false;
                break;
            }
        }
    }

    // CLOSE TIFF FILE
    TIFFClose(outputTiff);
    return (flag);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUMemoryObject::save(TIFF *otTiff, unsigned short index)
{
    // WRITE FORMAT PARAMETERS TO CURRENT TIFF DIRECTORY
    TIFFSetField(otTiff, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
    TIFFSetField(otTiff, TIFFTAG_IMAGEWIDTH, (unsigned long)width());
    TIFFSetField(otTiff, TIFFTAG_IMAGELENGTH, (unsigned long)(height()*frames()));
    TIFFSetField(otTiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    TIFFSetField(otTiff, TIFFTAG_XRESOLUTION, 72.0);
    TIFFSetField(otTiff, TIFFTAG_YRESOLUTION, 72.0);
    TIFFSetField(otTiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(otTiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(otTiff, TIFFTAG_SAMPLESPERPIXEL, (unsigned short)colors());
    TIFFSetField(otTiff, TIFFTAG_BITSPERSAMPLE, (unsigned short)(8 * depth()));
    TIFFSetField(otTiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    TIFFSetField(otTiff, TIFFTAG_XPOSITION, qMax(0.0f, (float)anchorPt.x()));
    TIFFSetField(otTiff, TIFFTAG_YPOSITION, qMax(0.0f, (float)anchorPt.y()));
    TIFFSetField(otTiff, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
    TIFFSetField(otTiff, TIFFTAG_ROWSPERSTRIP, 1);

    if (colors() == 3) {
        TIFFSetField(otTiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    } else if (colors() >= 4) {
        TIFFSetField(otTiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
    } else {
        TIFFSetField(otTiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    }

    if (depth() == sizeof(float)) {
        // SEE IF WE HAVE TO TELL THE TIFF READER THAT WE ARE STORING
        // PIXELS IN 32-BIT FLOATING POINT FORMAT
        TIFFSetField(otTiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
    }

    // MAKE TEMPORARY BUFFER TO HOLD CURRENT ROW BECAUSE COMPRESSION DESTROYS
    // WHATS EVER INSIDE THE BUFFER
    unsigned char *tempBuffer = (unsigned char *)malloc(step());
    for (unsigned int row = 0; row < height()*frames(); row++) {
        memcpy(tempBuffer, constScanLine(row), step());
        TIFFWriteScanline(otTiff, tempBuffer, row, 0);
    }
    free(tempBuffer);

    // WRITE THE CURRENT DIRECTORY AND PREPARE FOR THE NEW ONE
    TIFFWriteDirectory(otTiff);

    // WRITE THE ELAPSED TIME STAMP TO AN EXIF TAG
    uint64 dir_offset;
    TIFFCreateEXIFDirectory(otTiff);
    TIFFSetField(otTiff, EXIFTAG_SUBSECTIME, QString("%1").arg(elapsed()).toLatin1().data());
    TIFFWriteCustomDirectory(otTiff, &dir_offset);

    TIFFSetDirectory(otTiff, index);
    TIFFSetField(otTiff, TIFFTAG_EXIFIFD, dir_offset);
    TIFFRewriteDirectory(otTiff);

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUMemoryObject::load(TIFF *inTiff, unsigned short index)
{
    TIFFSetDirectory(inTiff, index);

    // LOAD INPUT TIFF FILE PARAMETERS IMPORTANT TO RESAMPLING THE IMAGE
    unsigned long uLongVariable;
    unsigned short uShortVariable;

    // NUMBER OF FRAMES IS ALWAYS EQUAL TO ONE
    data->numFrms = 1;

    // GET THE HEIGHT AND WIDTH OF INPUT IMAGE IN PIXELS
    TIFFGetField(inTiff, TIFFTAG_IMAGEWIDTH, &uLongVariable);
    data->numCols = uLongVariable;
    TIFFGetField(inTiff, TIFFTAG_IMAGELENGTH, &uLongVariable);
    data->numRows = uLongVariable;
    TIFFGetField(inTiff, TIFFTAG_SAMPLESPERPIXEL, &uShortVariable);
    data->numChns = uShortVariable;
    TIFFGetField(inTiff, TIFFTAG_BITSPERSAMPLE, &uShortVariable);
    data->numByts = uShortVariable / 8;

    // READ AND CHECK THE PHOTOMETRIC INTERPRETATION FIELD AND MAKE SURE ITS WHAT WE EXPECT
    TIFFGetField(inTiff, TIFFTAG_PHOTOMETRIC, &uShortVariable);

    if (colors() == 3) {
        if (uShortVariable != PHOTOMETRIC_RGB && uShortVariable != PHOTOMETRIC_MINISBLACK) {
            return (false);
        }
    } else if (colors() >= 4) {
        if (uShortVariable != PHOTOMETRIC_SEPARATED && uShortVariable != PHOTOMETRIC_MINISBLACK) {
            return (false);
        }
    } else {
        if (uShortVariable != PHOTOMETRIC_MINISWHITE && uShortVariable != PHOTOMETRIC_MINISBLACK) {
            return (false);
        }
    }

    // LOAD THE ANCHOR POINT
    float xPos = -1.0f, yPos = -1.0f;
    TIFFGetField(inTiff, TIFFTAG_XPOSITION, &xPos);
    TIFFGetField(inTiff, TIFFTAG_YPOSITION, &yPos);
    anchorPt = QPoint(qRound(xPos), qRound(yPos));

    // ALLOCATE SPACE TO HOLD IMAGE DATA
    data->allocateBuffer();

    // READ DATA AS EITHER CHUNKY OR PLANAR FORMAT
    if (data->buffer) {
        short shortVariable;
        TIFFGetField(inTiff, TIFFTAG_PLANARCONFIG, &shortVariable);
        if (shortVariable == PLANARCONFIG_SEPARATE) {
            unsigned char *tempBuffer = new unsigned char [step()];
            for (unsigned int chn = 0; chn < colors(); chn++) {
                for (unsigned int row = 0; row < height(); row++) {
                    unsigned char *pBuffer = scanLine(row);
                    TIFFReadScanline(inTiff, tempBuffer, (int)row, (int)chn);
                    for (unsigned int col = 0; col < width(); col++) {
                        ((float *)pBuffer)[col * colors() + chn] = ((float *)tempBuffer)[col];
                    }
                }
            }
            delete [] tempBuffer;
        } else if (shortVariable == PLANARCONFIG_CONTIG) {
            for (unsigned int row = 0; row < height(); row++) {
                TIFFReadScanline(inTiff, (unsigned char *)scanLine(row), (int)row);
            }
        }
    }

    // GET THE ELAPSED TIME VALUE FROM THE EXIF TAG FOR SUBSECOND TIME
    uint64 directoryOffset;
    if (TIFFGetField(inTiff, TIFFTAG_EXIFIFD, &directoryOffset)) {
        char *byteArray;
        TIFFReadEXIFDirectory(inTiff, directoryOffset);
        if (TIFFGetField(inTiff, EXIFTAG_SUBSECTIME, &byteArray)) {
            setElapsed(QString(QByteArray(byteArray)).toInt());
        }
    }

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
unsigned int LAUMemoryObject::nonZeroPixelsCount() const
{
    // CREATE A REGISTER TO HOLD THE PIXEL COUNT
    unsigned int pixels = 0;

    // CREATE A VECTOR TO HOLD THE ACCUMULATED SUM OF PIXELS
    __m128i acSum = _mm_set1_epi32(0);

    if (depth() == sizeof(unsigned char)) {
        // CREATE A ZERO VECTOR FOR THE COMPARE OPERATION
        __m128i zeros = _mm_set1_epi8(0);

        // GRAB THE POINTER TO THE MEMORY OBJECT DATA
        unsigned char *buffer = (unsigned char *)constPointer();

        // ITERATE THROUGH THE BUFFER 16 BYTES AT A TIME
        for (unsigned int n = 0; n < length(); n += 16) {
            __m128i pixels = _mm_cmpeq_epi8(_mm_load_si128((const __m128i *)(buffer + n)), zeros);

            // HORIZONTAL SUM THE BYTES INTO 64-BIT INTEGERS AND
            __m128i vecL = _mm_sad_epu8(pixels, zeros);

            // ACCUMULATE THE SUM OF THE VECTORS TO FORM A SUM OF INTS
            acSum = _mm_add_epi32(acSum, vecL);
        }
        acSum = _mm_hadd_epi32(acSum, acSum);
        acSum = _mm_hadd_epi32(acSum, acSum);

        // EXTRACT THE INTEGER AND DIVIDE BY 255 TO GET UNITS OF PIXELS
        pixels = _mm_extract_epi32(acSum, 0) / 255;
    } else if (depth() == sizeof(unsigned short)) {
        // CREATE A ZERO VECTOR FOR THE COMPARE OPERATION
        __m128i zeros = _mm_set1_epi16(0);

        // GRAB THE POINTER TO THE MEMORY OBJECT DATA
        unsigned char *buffer = (unsigned char *)constPointer();

        // ITERATE THROUGH THE BUFFER 16 BYTES AT A TIME
        for (unsigned int n = 0; n < length(); n += 16) {
            __m128i pixels = _mm_cmpeq_epi16(_mm_load_si128((const __m128i *)(buffer + n)), zeros);

            // UNPACK FROM UNSIGNED SHORTS TO UNSIGNED INTS
            __m128i vecL = _mm_hadds_epi16(pixels, zeros);
            __m128i vecH = _mm_cvtepi16_epi32(vecL);

            // ACCUMULATE THE SUM OF THE VECTORS TO FORM A SUM OF INTS
            acSum = _mm_add_epi32(acSum, vecH);
        }
        acSum = _mm_hadd_epi32(acSum, acSum);
        acSum = _mm_hadd_epi32(acSum, acSum);

        // EXTRACT THE INTEGER AND DIVIDE BY 65535 TO GET UNITS OF PIXELS
        pixels = (unsigned int)(-1 * _mm_extract_epi32(acSum, 0));
    } else if (depth() == sizeof(float)) {
        // CREATE A ZERO VECTOR FOR THE COMPARE OPERATION
        __m128 zeros = _mm_set1_ps(0.0f);

        // GRAB THE POINTER TO THE MEMORY OBJECT DATA
        unsigned char *buffer = (unsigned char *)constPointer();

        // ITERATE THROUGH THE BUFFER 16 BYTES AT A TIME
        for (unsigned int n = 0; n < length(); n += 16) {
            __m128i pixels = _mm_castps_si128(_mm_cmpeq_ps(_mm_load_ps((const float *)(buffer + n)), zeros));

            // ACCUMULATE THE SUM OF THE VECTORS TO FORM A SUM OF INTS
            acSum = _mm_add_epi32(acSum, pixels);
        }
        acSum = _mm_hadd_epi32(acSum, acSum);
        acSum = _mm_hadd_epi32(acSum, acSum);

        // EXTRACT THE INTEGER AND DIVIDE BY 65535 TO GET UNITS OF PIXELS
        pixels = (unsigned int)(-1 * _mm_extract_epi32(acSum, 0));
    }

    // AT THIS POINT, THE SUM OF ZEROS RESULTS IN ADDING -1S TOGETHER
    // SO WE JUST NEED TO ADD THE NUMBER OF PIXELS TO GET THE NUMBER
    // OF NON-ZERO PIXELS IN THE BUFFER
    pixels = (unsigned int)((int)(width() * height() * colors()) - (int)pixels);

    // RETURN THE NUMBER OF NON-ZERO PIXELS
    return (pixels);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAUMemoryObject::average(QList<LAUMemoryObject> objects)
{
    LAUMemoryObject result = objects.first();
    LAUMemoryObject intermediate = LAUMemoryObject(result.width(), result.height(), result.colors(), sizeof(unsigned int), result.frames());
    memset(intermediate.constPointer(), 0, intermediate.length());

    if (result.depth() == sizeof(unsigned char)) {
        // GRAB A COPY OF THE INTERMEDIATE BUFFER
        unsigned int *csBuffer = (unsigned int *)intermediate.constPointer();

        // ITERATE THROUGH ALL AVAILABLE FRAMES
        for (int n = 0; n < objects.count(); n++) {
            unsigned char *inBuffer = (unsigned char *)objects.at(n).constPointer();

            // ADD IN THE CURRENT FRAME TO OUR CUMMULATE SUM OBJECT
            for (unsigned int k = 0; k < intermediate.length() / sizeof(unsigned int); k++) {
                csBuffer[k] += (unsigned int)inBuffer[k];
            }
        }

        // CALCULATE THE MEAN PIXEL VALUE ACROSS ALL FRAMES
        unsigned char *otBuffer = (unsigned char *)result.pointer();
        for (unsigned int k = 0; k < intermediate.length() / sizeof(unsigned int); k++) {
            otBuffer[k] = (unsigned char)(csBuffer[k] / objects.count());
        }
    } else if (result.depth() == sizeof(unsigned short)) {
        // GRAB A COPY OF THE INTERMEDIATE BUFFER
        unsigned int *csBuffer = (unsigned int *)intermediate.constPointer();

        // ITERATE THROUGH ALL AVAILABLE FRAMES
        for (int n = 0; n < objects.count(); n++) {
            unsigned short *inBuffer = (unsigned short *)objects.at(n).constPointer();

            // ADD IN THE CURRENT FRAME TO OUR CUMMULATE SUM OBJECT
            for (unsigned int k = 0; k < intermediate.length() / sizeof(unsigned int); k++) {
                csBuffer[k] += (unsigned int)inBuffer[k];
            }
        }

        // CALCULATE THE MEAN PIXEL VALUE ACROSS ALL FRAMES
        unsigned short *otBuffer = (unsigned short *)result.pointer();
        for (unsigned int k = 0; k < intermediate.length() / sizeof(unsigned int); k++) {
            otBuffer[k] = (unsigned short)(csBuffer[k] / objects.count());
        }
    } else if (result.depth() == sizeof(float)) {
        // GRAB A COPY OF THE INTERMEDIATE BUFFER
        float *csBuffer = (float *)intermediate.constPointer();

        // ITERATE THROUGH ALL AVAILABLE FRAMES
        for (int n = 0; n < objects.count(); n++) {
            float *inBuffer = (float *)objects.at(n).constPointer();

            // ADD IN THE CURRENT FRAME TO OUR CUMMULATE SUM OBJECT
            for (unsigned int k = 0; k < intermediate.length() / sizeof(float); k++) {
                csBuffer[k] += inBuffer[k];
            }
        }

        // CALCULATE THE MEAN PIXEL VALUE ACROSS ALL FRAMES
        float *otBuffer = (float *)result.pointer();
        for (unsigned int k = 0; k < intermediate.length() / sizeof(unsigned int); k++) {
            otBuffer[k] = (float)(csBuffer[k] / objects.count());
        }
    }

    // RETURN THE AVERAGE SCAN TO THE USER
    return (result);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectManager::~LAUMemoryObjectManager()
{
    framesAvailable.clear();
    qDebug() << QString("LAUMemoryObjectManager::~LAUMemoryObjectManager()");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMemoryObjectManager::onGetFrame()
{
    // CREATE NEW BUFFER IF NONE AVAILABLE, OTHERWISE TAKE FIRST AVAILABLE
    if (framesAvailable.isEmpty()) {
        emit emitFrame(LAUMemoryObject(numCols, numRows, numChns, numByts));
    } else {
        emit emitFrame(framesAvailable.takeFirst());
    }

    // NOW CREATE TWO NEW BUFFERS TO EVERY BUFFER RELEASED UP UNTIL
    // WE HAVE A MINIMUM NUMBER OF BUFFERS AVAILABLE FOR SUBSEQUENT CALLS
    if (framesAvailable.count() < MINNUMBEROFFRAMESAVAILABLE) {
        framesAvailable << LAUMemoryObject(numCols, numRows, numChns, numByts);
        framesAvailable << LAUMemoryObject(numCols, numRows, numChns, numByts);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMemoryObjectManager::onReleaseFrame(LAUMemoryObject frame)
{
    // WE CAN EITHER ADD THE BUFFER BACK TO THE AVAILABLE LIST, IF THERE IS SPACE
    // OR OTHERWISE FREE THE MEMORY
    if (framesAvailable.count() < MAXNUMBEROFFRAMESAVAILABLE) {
        framesAvailable << frame;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectWriter::LAUMemoryObjectWriter(QString filename, LAUMemoryObject obj, QObject *parent) : QThread(parent), tiff(nullptr), object(obj)
{
    if (object.isValid()) {
        // LET THE USER SELECT A FILE FROM THE FILE DIALOG
        if (filename.isNull()) {
#ifndef HEADLESS
            QSettings settings;
            QString directory = settings.value("LAUMemoryObject::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
            filename = QFileDialog::getSaveFileName(nullptr, QString("Save image to disk (*.tif)"), directory);
            if (filename.isEmpty() == false) {
                if (filename.toLower().endsWith(".tif") == false && filename.toLower().endsWith(".tiff") == false) {
                    filename.append(".tif");
                }
                settings.setValue("LAUMemoryObject::lastUsedDirectory", QFileInfo(filename).absolutePath());
            } else {
                return;
            }
#else
            return;
#endif
        }

        // OPEN TIFF FILE FOR SAVING THE IMAGE USING BIGTIFF FOR LARGE FILES
        tiff = (object.length() > 100000000) ? TIFFOpen(filename.toLatin1(), "w8") : TIFFOpen(filename.toLatin1(), "w");
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObjectWriter::~LAUMemoryObjectWriter()
{
    qDebug() << QString("LAUMemoryObjectWriter::~LAUMemoryObjectWriter()");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMemoryObjectWriter::run()
{
    // MAKE SURE WE HAVE A VALID FILE TO WRITE TO
    if (tiff) {
        for (unsigned int frm = 0; frm < object.frames(); frm++) {
            // WRITE FORMAT PARAMETERS TO CURRENT TIFF DIRECTORY
            TIFFSetField(tiff, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
            TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, (unsigned long)object.width());
            TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, (unsigned long)object.height());
            TIFFSetField(tiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
            TIFFSetField(tiff, TIFFTAG_XRESOLUTION, 72.0);
            TIFFSetField(tiff, TIFFTAG_YRESOLUTION, 72.0);
            TIFFSetField(tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
            TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
            TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, (unsigned short)object.colors());
            TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, (unsigned short)(8 * object.depth()));
            TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
            TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
            TIFFSetField(tiff, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
            TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, 1);

            if (object.depth() == sizeof(float)) {
                // SEE IF WE HAVE TO TELL THE TIFF READER THAT WE ARE STORING
                // PIXELS IN 32-BIT FLOATING POINT FORMAT
                TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
            }

            // MAKE TEMPORARY BUFFER TO HOLD CURRENT ROW BECAUSE COMPRESSION DESTROYS
            // WHATS EVER INSIDE THE BUFFER
            unsigned char *tempBuffer = (unsigned char *)malloc(object.step());
            for (unsigned int row = 0; row < object.height(); row++) {
                memcpy(tempBuffer, object.constScanLine(row, frm), object.step());
                TIFFWriteScanline(tiff, tempBuffer, row, 0);
            }
            free(tempBuffer);

            // WRITE THE CURRENT DIRECTORY AND PREPARE FOR THE NEW ONE
            TIFFWriteDirectory(tiff);

            // WRITE THE ELAPSED TIME STAMP TO AN EXIF TAG
            uint64 dir_offset;
            TIFFCreateEXIFDirectory(tiff);
            TIFFSetField(tiff, EXIFTAG_SUBSECTIME, QString("%1").arg(object.elapsed()).toLatin1().data());
            TIFFWriteCustomDirectory(tiff, &dir_offset);

            TIFFSetDirectory(tiff, (unsigned short)frm);
            TIFFSetField(tiff, TIFFTAG_EXIFIFD, dir_offset);
            TIFFRewriteDirectory(tiff);
        }
        TIFFClose(tiff);
    }
    emit emitSaveComplete();
}
