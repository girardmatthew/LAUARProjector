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
#include "laulookuptable.h"
#include <locale.h>
#include <math.h>

int LAULookUpTableData::instanceCounter = 0;

using namespace libtiff;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULookUpTableData::LAULookUpTableData()
{
    filename = QString();
    xmlString = QString();
    makeString = QString();
    modelString = QString();
    softwareString = QString();
    style = StyleUndefined;

    xMin = 0.0f;
    xMax = 0.0f;
    yMin = 0.0f;
    yMax = 0.0f;
    zMin = 0.0f;
    zMax = 0.0f;
    pMin = 0.0f;
    pMax = 1.0f;

    numRows = 0;
    numCols = 0;
    numChns = 0;
    numSmps = 0;

    buffer = nullptr;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULookUpTableData::LAULookUpTableData(const LAULookUpTableData &other) : LAULookUpTableData()
{
    qDebug() << QString("Performing deep copy on %1").arg(filename);

    xMin = other.xMin;
    xMax = other.xMax;
    yMin = other.yMin;
    yMax = other.yMax;
    zMin = other.zMin;
    zMax = other.zMax;
    pMin = other.pMin;
    pMax = other.pMax;

    numRows = other.numRows;
    numCols = other.numCols;
    numChns = other.numChns;

    style = other.style;
    xmlString = other.xmlString;
    makeString = other.makeString;
    filename = other.filename;
    modelString = other.modelString;
    softwareString = other.softwareString;

    allocateBuffer();
    memcpy(buffer, other.buffer, numSmps * sizeof(float));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULookUpTableData::~LAULookUpTableData()
{
    qDebug() << QString("LAULookUpTableData::~LAULookUpTableData() %1").arg(--instanceCounter);
    if (buffer != nullptr) {
        _mm_free(buffer);
        buffer = nullptr;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAULookUpTableData::allocateBuffer()
{
    qDebug() << QString("LAULookUpTableData::allocateBuffer() %1").arg(instanceCounter++) << "Size: " << numRows << " x " << numCols;

    // ALLOCATE SPACE FOR HOLDING PIXEL DATA BASED ON NUMBER OF CHANNELS AND BYTES PER PIXEL
    numSmps  = (unsigned long long)numRows;
    numSmps *= (unsigned long long)numCols;
    numSmps *= (unsigned long long)numChns;

    if (numSmps) {
        buffer = _mm_malloc(numSmps * sizeof(float), 16);
        if (buffer == nullptr) {
            qDebug() << QString("LAULookUpTableData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
            qDebug() << QString("LAULookUpTableData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
            qDebug() << QString("LAULookUpTableData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
        }
    } else {
        buffer = nullptr;
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULookUpTable::LAULookUpTable(unsigned int cols, unsigned int rows, LAUVideoPlaybackDevice device, float hFov, float vFov, float zMin, float zMax, float pMin, float pMax)
{
    data = new LAULookUpTableData();

    if (device == DeviceProsilicaGRY) {
        return;
    }

    data->numRows = rows;
    data->numCols = cols;
    data->numChns = 12;
    data->allocateBuffer();

    // THE DEFAULT LOOK UP TABLE IS A FOURTH ORDER POLYNOMIAL
    data->style = StyleFourthOrderPoly;

    data->xMin = -1.2f;
    data->xMax = 1.2f;
    data->yMin = -1.2f;
    data->yMax = 1.2f;
    data->zMin = -qMax(qAbs(zMax), qAbs(zMin));
    data->zMax = -qMin(qAbs(zMax), qAbs(zMin));
    data->pMin = pMin;
    data->pMax = pMax;

    data->verticalFieldOfView = vFov;
    data->horizontalFieldOfView = hFov;

    if (rows * cols > 0) {
        // CREATE BUFFER TO HOLD ABCD AND EFGH TEXTURES
        float *buffer = (float *)data->buffer;

        // INITIALIZE ABCDEFGH COEFFICIENTS
        if (device == DeviceProsilicaIOS || device == DeviceProsilicaLCG || device == DeviceProsilicaDPR) {
            // SET THE Z LIMITS AND CALCULATE THE FIELD OF VIEW
            data->pMin = 0.0f;
            data->pMax = 1.0f;
            data->zMin = -110.0f;
            data->zMax = -90.0f;
            data->yMin = -(float)(data->numRows / 2);
            data->yMax = -data->yMin;
            data->xMin = -(float)(data->numCols / 2);
            data->xMax = -data->xMin;

            float   phiA = atan(data->yMin / data->zMin);
            float   phiB = atan(data->yMax / data->zMin);
            float thetaA = atan(data->xMin / data->zMin);
            float thetaB = atan(data->xMax / data->zMin);

            data->horizontalFieldOfView = fabs(thetaA) + fabs(thetaB);
            data->verticalFieldOfView = fabs(phiA) + fabs(phiB);

            float lftEdge = -100.0f * tan(data->horizontalFieldOfView / 2.0f);
            float rghEdge = -lftEdge;
            float topEdge =  100.0f * tan(data->horizontalFieldOfView / 2.0f);
            float btmEdge = -topEdge;

            int index = 0;
            for (unsigned int row = 0; row < data->numRows; row++) {
                for (unsigned int col = 0; col < data->numCols; col++) {
                    float lambdaX = (float)col / (float)(data->numCols - 1);
                    float lambdaY = (float)row / (float)(data->numRows - 1);

                    // DEFINE THE Z TO XY LINEAR COEFFICIENTS
                    buffer[index++] =   0.0f; // A
                    buffer[index++] = lftEdge * (1.0f - lambdaX) + rghEdge * lambdaX; // B
                    buffer[index++] =   0.0f; // C
                    buffer[index++] = btmEdge * (1.0f - lambdaY) + topEdge * lambdaY; // D

                    // DEFINE THE COEFFICIENTS FOR A FOURTH ORDER POLYNOMIAL
                    buffer[index++] =    0.0f;   // E
                    buffer[index++] =    0.0f;   // F
                    buffer[index++] =    0.0f;   // G
                    buffer[index++] =    0.0f;   // H
                    buffer[index++] = -100.0f;   // I
                    buffer[index++] =     NAN;   // F
                    buffer[index++] =     NAN;   // G
                    buffer[index++] =     NAN;   // H
                }
            }
        } else if (device == DeviceKinect) {
            // SET THE Z LIMITS AND CALCULATE THE FIELD OF VIEW
            data->pMin = 0.0f;
            data->pMax = 1.0f;
            data->yMin = tan(data->verticalFieldOfView / 2.0f) * data->zMin;
            data->yMax = -data->yMin;
            data->xMin = tan(data->horizontalFieldOfView / 2.0f) * data->zMin;
            data->xMax = -data->xMin;

            int index = 0;
            for (unsigned int row = 0; row < data->numRows; row++) {
                for (unsigned int col = 0; col < data->numCols; col++) {
                    // DEFINE THE Z TO XY LINEAR COEFFICIENTS
                    buffer[index++] = tan((((float)col + 0.5f) / (float)(data->numCols) - 0.5f) * data->horizontalFieldOfView); // A
                    buffer[index++] =    0.0f; // B
                    buffer[index++] = tan((((float)row + 0.5f) / (float)(data->numRows) - 0.5f) * data->verticalFieldOfView); // C
                    buffer[index++] =    0.0f; // D

                    // DEFINE THE COEFFICIENTS FOR A FOURTH ORDER POLYNOMIAL
                    buffer[index++] =      0.0f; // E
                    buffer[index++] =      0.0f; // F
                    buffer[index++] =      0.0f; // G
#ifdef AZUREKINECT
                    buffer[index++] = -65535.0f; // H
                    buffer[index++] =      0.0f; // I
#else
                    buffer[index++] = -65130.7f; // H
                    buffer[index++] =    -20.0f; // I
#endif
                    buffer[index++] =       NAN;
                    buffer[index++] =       NAN;
                    buffer[index++] =       NAN;
                }
            }
        } else if (device == DevicePrimeSense) {
            // SET THE Z LIMITS AND CALCULATE THE FIELD OF VIEW
            data->pMin = 0.0f;
            data->pMax = 1.0f;
            data->yMin = tan(data->verticalFieldOfView / 2.0f) * data->zMin;
            data->yMax = -data->yMin;
            data->xMin = tan(data->horizontalFieldOfView / 2.0f) * data->zMin;
            data->xMax = -data->xMin;

            int index = 0;
            for (unsigned int row = 0; row < data->numRows; row++) {
                for (unsigned int col = 0; col < data->numCols; col++) {
                    // DEFINE THE Z TO XY LINEAR COEFFICIENTS
                    buffer[index++] = tan((((float)col + 0.5f) / (float)(data->numCols) - 0.5f) * data->horizontalFieldOfView); // A
                    buffer[index++] = 0.0f; // B
                    buffer[index++] = tan((((float)row + 0.5f) / (float)(data->numRows) - 0.5f) * data->verticalFieldOfView); // C
                    buffer[index++] = 0.0f; // D

                    // DEFINE THE COEFFICIENTS FOR A FOURTH ORDER POLYNOMIAL
                    buffer[index++] =      0.0f; // E
                    buffer[index++] =      0.0f; // F
                    buffer[index++] =      0.0f; // G
                    buffer[index++] =  -6185.7f; // H
                    buffer[index++] =    -62.0f; // I
                    buffer[index++] =       NAN;
                    buffer[index++] =       NAN;
                    buffer[index++] =       NAN;
                }
            }
        } else if (device == DeviceRealSense) {
            // SET THE Z LIMITS AND CALCULATE THE FIELD OF VIEW
            data->pMin = 0.0f;
            data->pMax = 1.0f;
            data->yMin = tan(data->verticalFieldOfView / 2.0f) * data->zMin;
            data->yMax = -data->yMin;
            data->xMin = tan(data->horizontalFieldOfView / 2.0f) * data->zMin;
            data->xMax = -data->xMin;

            int index = 0;
            for (unsigned int row = 0; row < data->numRows; row++) {
                for (unsigned int col = 0; col < data->numCols; col++) {
                    // DEFINE THE Z TO XY LINEAR COEFFICIENTS
                    buffer[index++] = -tan((((float)col + 0.5f) / (float)(data->numCols) - 0.5f) * data->horizontalFieldOfView); // A
                    buffer[index++] =    0.0f; // B
                    buffer[index++] =  tan((((float)row + 0.5f) / (float)(data->numRows) - 0.5f) * data->verticalFieldOfView); // C
                    buffer[index++] =    0.0f; // D

                    // DEFINE THE COEFFICIENTS FOR A FOURTH ORDER POLYNOMIAL
                    buffer[index++] =      0.0f; // E
                    buffer[index++] =      0.0f; // F
                    buffer[index++] =      0.0f; // G
                    buffer[index++] = -65535.0f; // H
                    buffer[index++] =      0.0f; // I
                    buffer[index++] =       NAN;
                    buffer[index++] =       NAN;
                    buffer[index++] =       NAN;
                }
            }
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULookUpTable::LAULookUpTable(QString filename)
{
    // CREATE OUR DATA OBJECT TO HOLD THE SCAN
    data = new LAULookUpTableData();

#ifndef USEHEADLESS
    // GET A FILE TO OPEN FROM THE USER IF NOT ALREADY PROVIDED ONE
    if (filename.isNull()) {
        QSettings settings;
        QString directory = settings.value(QString("LAULookUpTable::LastUsedDirectory"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        if (QDir().exists(directory) == false) {
            directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        }
        filename = QFileDialog::getOpenFileName(nullptr, QString("Load scan from disk (*.lut)"), directory, QString("*.lut"));
        if (!filename.isNull()) {
            settings.setValue(QString("LAULookUpTable::LastUsedDirectory"), QFileInfo(filename).absolutePath());
        } else {
            return;
        }
    }
#else
    if (filename.isNull()) {
        return;
    }
#endif
    // IF WE HAVE A VALID TIFF FILE, LOAD FROM DISK
    // OTHERWISE TRY TO CONNECT TO SCANNER
    if (QFile::exists(filename)) {
        // SET THE FILENAME
        setFilename(filename);

        // OPEN INPUT TIFF FILE FROM DISK
        TIFF *inTiff = TIFFOpen(filename.toLatin1(), "r");
        if (inTiff) {
            load(inTiff);
            TIFFClose(inTiff);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULookUpTable::LAULookUpTable(TIFF *currentTiffDirectory)
{
    data = new LAULookUpTableData();
    load(currentTiffDirectory);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULookUpTable::~LAULookUpTable()
{
    qDebug() << "LAULookUpTable::~LAULookUpTable()";
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAULookUpTable::save(QString filename)
{
#ifndef USEHEADLESS
    if (filename.isNull()) {
        QSettings settings;
        QString directory = settings.value(QString("LAULookUpTable::LastUsedDirectory"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        if (QDir().exists(directory) == false) {
            directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        }
        filename = QFileDialog::getSaveFileName(nullptr, QString("Save look-up table to disk (*.lut)"), directory, QString("*.lut"));
        if (!filename.isNull()) {
            settings.setValue(QString("LAULookUpTable::LastUsedDirectory"), QFileInfo(filename).absolutePath());
            if (!filename.toLower().endsWith(QString(".lut"))) {
                if (!filename.toLower().endsWith(QString(".lut"))) {
                    filename = QString("%1.lut").arg(filename);
                }
            }
        } else {
            return (false);
        }
    }
#else
    if (filename.isNull()) {
        return (false);
    }
#endif

    // OPEN TIFF FILE FOR SAVING THE IMAGE
    TIFF *outputTiff = TIFFOpen(filename.toLatin1(), "w8");
    if (!outputTiff) {
        return (false);
    }

    // WRITE IMAGE TO CURRENT DIRECTORY
    save(outputTiff);

    // CLOSE TIFF FILE
    TIFFClose(outputTiff);

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAULookUpTable::save(TIFF *currentTiffDirectory)
{
    // WRITE FORMAT PARAMETERS TO CURRENT TIFF DIRECTORY
    TIFFSetField(currentTiffDirectory, TIFFTAG_IMAGEWIDTH, (unsigned long)width());
    TIFFSetField(currentTiffDirectory, TIFFTAG_IMAGELENGTH, (unsigned long)height());
    TIFFSetField(currentTiffDirectory, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    TIFFSetField(currentTiffDirectory, TIFFTAG_XRESOLUTION, 72.0);
    TIFFSetField(currentTiffDirectory, TIFFTAG_YRESOLUTION, 72.0);
    TIFFSetField(currentTiffDirectory, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(currentTiffDirectory, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(currentTiffDirectory, TIFFTAG_SAMPLESPERPIXEL, (unsigned short)colors());
    TIFFSetField(currentTiffDirectory, TIFFTAG_BITSPERSAMPLE, (unsigned short)(8 * sizeof(float)));
    TIFFSetField(currentTiffDirectory, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
#ifndef _TTY_WIN_
    TIFFSetField(currentTiffDirectory, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    TIFFSetField(currentTiffDirectory, TIFFTAG_PREDICTOR, 2);
    TIFFSetField(currentTiffDirectory, TIFFTAG_ROWSPERSTRIP, 1);
#endif

    // SEE IF WE HAVE TO TELL THE TIFF READER THAT WE ARE STORING
    // PIXELS IN 32-BIT FLOATING POINT FORMAT
    TIFFSetField(currentTiffDirectory, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);

    // MAKE TEMPORARY BUFFER TO HOLD CURRENT ROW BECAUSE COMPRESSION DESTROYS
    // WHATS EVER INSIDE THE BUFFER
    unsigned char *tempBuffer = (unsigned char *)malloc(step());
    for (unsigned int row = 0; row < height(); row++) {
        memcpy(tempBuffer, constScanLine(row), step());
        TIFFWriteScanline(currentTiffDirectory, tempBuffer, row, 0);
    }
    free(tempBuffer);

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAULookUpTable::load(TIFF *inTiff)
{
    // LOAD INPUT TIFF FILE PARAMETERS IMPORTANT TO RESAMPLING THE IMAGE
    unsigned long uLongVariable;
    unsigned short uShortVariable;

    // SEE HOW MANY DIRECTORIES ARE IN THE TIFF FILE
    unsigned short directories = TIFFNumberOfDirectories(inTiff);

    // GET THE HEIGHT AND WIDTH OF INPUT IMAGE IN PIXELS
    TIFFGetField(inTiff, TIFFTAG_IMAGEWIDTH, &uLongVariable);
    data->numCols = static_cast<unsigned int>(uLongVariable);
    TIFFGetField(inTiff, TIFFTAG_IMAGELENGTH, &uLongVariable);
    data->numRows = static_cast<unsigned int>(uLongVariable);
    TIFFGetField(inTiff, TIFFTAG_SAMPLESPERPIXEL, &uShortVariable);
    if (directories == 1) {
        if (uShortVariable == 8) {
            data->style = StyleLinear;
            data->numChns = uShortVariable;
        } else if (uShortVariable == 12) {
            data->style = StyleFourthOrderPoly;
            data->numChns = uShortVariable;
        } else if (uShortVariable == 16) {
            data->style = StyleFourthOrderPolyAugmentedReality;
            data->numChns = uShortVariable;
        } else {
            return (false);
        }
    } else if (directories == 2) {
        if (uShortVariable == 12) {
            data->style = StyleFourthOrderPolyWithPhaseUnwrap;
            data->numChns = uShortVariable + 1;
        } else {
            return (false);
        }
    } else {
        if (uShortVariable == 4) {
            data->style = StyleXYZPLookUpTable;
            data->numChns = uShortVariable * directories;
        } else if (uShortVariable == 8) {
            data->style = StyleXYZWRCPQLookUpTable;
            data->numChns = uShortVariable * directories;
        } else {
            return (false);
        }
    }

    TIFFGetField(inTiff, TIFFTAG_BITSPERSAMPLE, &uShortVariable);
    if (uShortVariable != 32) {
        return (false);
    }

    TIFFGetField(inTiff, TIFFTAG_PHOTOMETRIC, &uShortVariable);
    if (uShortVariable != PHOTOMETRIC_MINISBLACK) {
        return (false);
    }

    TIFFGetField(inTiff, TIFFTAG_SAMPLEFORMAT, &uShortVariable);
    if (uShortVariable != SAMPLEFORMAT_IEEEFP) {
        return (false);
    }

    TIFFGetField(inTiff, TIFFTAG_PLANARCONFIG, &uShortVariable);
    if (uShortVariable != PLANARCONFIG_CONTIG) {
        return (false);
    }

    // GET THE Z LIMITS AND FLIP THEIR SIGNS IF NECESSARY
    TIFFGetField(inTiff, TIFFTAG_MINSAMPLEVALUE, &uShortVariable);
    data->zMin = (float)uShortVariable;

    TIFFGetField(inTiff, TIFFTAG_MAXSAMPLEVALUE, &uShortVariable);
    data->zMax = (float)uShortVariable;

    double doubleVariable = 0.0;
    TIFFGetField(inTiff, TIFFTAG_SMINSAMPLEVALUE, &doubleVariable);
    if (doubleVariable != 0.0) {
        data->zMin = (float)doubleVariable;
        data->pMin = (float)doubleVariable;
    }

    TIFFGetField(inTiff, TIFFTAG_SMAXSAMPLEVALUE, &doubleVariable);
    if (doubleVariable != 0.0) {
        data->zMax = (float)doubleVariable;
        data->pMax = (float)doubleVariable;
    }

    if (data->zMin > data->zMax) {
        data->zMin *= -1.0f;
        data->zMax *= -1.0f;
    }

    int dataLength;
    char *dataString;
    bool dataPresent = TIFFGetField(inTiff, TIFFTAG_XMLPACKET, &dataLength, &dataString);
    if (dataPresent) {
        data->xmlString = QString(QByteArray(dataString));
    }

    dataPresent = TIFFGetField(inTiff, TIFFTAG_MODEL, &dataString);
    if (dataPresent) {
        data->modelString = QString(QByteArray(dataString));
    }

    dataPresent = TIFFGetField(inTiff, TIFFTAG_SOFTWARE, &dataString);
    if (dataPresent) {
        data->softwareString = QString(QByteArray(dataString));
    }

    dataPresent = TIFFGetField(inTiff, TIFFTAG_MAKE, &dataString);
    if (dataPresent) {
        data->makeString = QString(QByteArray(dataString));
    }

    // ALLOCATE SPACE TO HOLD IMAGE DATA
    data->allocateBuffer();

    if (directories == 1) {
        // READ DATA AS CHUNKY FORMAT
        for (unsigned int row = 0; row < height(); row++) {
            unsigned char *buffer = scanLine(row);
            TIFFReadScanline(inTiff, buffer, static_cast<libtiff::uint32>(row));
        }
    } else if (directories == 2) {
        // CALCULATE MEMORY STEP SIZES ASSUMING 12 FLOATS PER PIXEL
        unsigned int bytesPerRow = (data->numCols * 12 * sizeof(float));
        unsigned int bytesPerFrame = (data->numRows * bytesPerRow);

        unsigned char *buffer = (unsigned char *)scanLine(0);
        for (unsigned int row = 0; row < height(); row++) {
            TIFFReadScanline(inTiff, (buffer + row * bytesPerRow), static_cast<libtiff::uint32>(row));
        }

        // MOVE THE POINTER TO THE NEXT AVAILABLE ADDRESS
        buffer += bytesPerFrame;

        // AT THIS POINT, WE HAVE JUST ENOUGH ROOM FOR 1 FLOAT PER PIXEL
        // SO LET'S GO TO THE NEXT DIRECTORY TO LOAD THIS UNWRAPPING MASK
        LAUMemoryObject unwrapMaskObject(inTiff, 1);
        if (unwrapMaskObject.isValid()) {
            memcpy(buffer, unwrapMaskObject.constPointer(), unwrapMaskObject.length());
        }
    } else {
        // CALCULATE MEMORY STEP SIZES
        unsigned int bytesPerRow = (data->numCols * sizeof(float));
        if (data->style == StyleXYZPLookUpTable) {
            bytesPerRow *= 4;
        } else if (data->style == StyleXYZWRCPQLookUpTable) {
            bytesPerRow *= 8;
        }
        unsigned int bytesPerFrame = (data->numRows * bytesPerRow);

#define USEMULTITHREADLOOKUPTABLELOADING
#ifdef USEMULTITHREADLOOKUPTABLELOADING
        // READ EACH DIRECTORY TO FORM A PLANAR IMAGE OF VEC4S
        QList<QThread *> loaders;
        for (unsigned short dir = 0; dir < directories; dir++) {
            // CREATE A LOADER OBJECT FOR THE CURRENT DIRECTORY
            QThread *thread = new LAULookUpTableLoader(filename(), dir, (unsigned char *)constScanLine(0) + bytesPerFrame * dir);
            thread->start();
            loaders << thread;
        }

        // NOW WAIT FOR ALL THREADS TO COMPLETE AND THEN DELETE THEM ONE BY ONE
        while (loaders.count() > 0) {
            QThread *thread = loaders.takeFirst();
            while (thread->isRunning()) {
                qApp->processEvents();
            }
            delete thread;
        }
#else
        // READ EACH DIRECTORY TO FORM A PLANAR IMAGE OF VEC4S
        unsigned char *buffer = (unsigned char *)scanLine(0);
        for (unsigned short dir = 0; dir < directories; dir++) {
            TIFFSetDirectory(inTiff, dir);
            for (unsigned int row = 0; row < height(); row++) {
                TIFFReadScanline(inTiff, (buffer + row * bytesPerRow), (int)row);
            }
            // MOVE THE POINTER TO THE NEXT AVAILABLE ADDRESS
            buffer += bytesPerFrame;
        }
#endif
    }

    // SET THE LIMITS OF THE LUT
    updateLimits();

    // IF WE MADE IT THIS FAR, EVERYTHING IS GOOD AND RETURN TRUE
    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAULookUpTable::replace(const LAULookUpTable &other)
{
    data = other.data;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULookUpTable LAULookUpTable::crop(unsigned int y, unsigned int x, unsigned int h, unsigned int w)
{
    if ((y + h) > height()) {
        h = height() - y;
    }
    if ((x + w) > width()) {
        w = width() - x;
    }

    LAULookUpTable image(h, w);

    for (unsigned int r = 0; r < image.height(); r++) {
        float *otBuffer = (float *)image.scanLine(r);
        float *inBuffer = (float *) & (((float *)this->constScanLine(y + r))[colors() * x + 0]);
        memcpy(otBuffer, inBuffer, w * sizeof(float)*colors());
    }

    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAULookUpTable::updateLimits()
{
    if (data->numChns == 12 || data->numChns == 13 || data->numChns == 16) {
        int offset = (data->numChns == 16) ? 16 : 12;

        // USE THE Z LIMITS TO FIND THE X AND Y LIMITS FROM THE ABCD COEFFICIENTS
        int index = 0;

        // INITIALIZE THE X AND Y LIMITS
        data->xMin = 0.0f;
        data->yMin = 0.0f;
        data->xMax = 0.0f;
        data->yMax = 0.0f;

        // ITERATE THROUGH EACH AND EVERY PIXEL
        for (unsigned int row = 0; row < data->numRows; row++) {
            for (unsigned int col = 0; col < data->numCols; col++) {
                // GET THE X RANGE FROM THE ZMIN VALUE
                float x = ((float *)data->buffer)[index + 0] * data->zMin + ((float *)data->buffer)[index + 1];
                if (x < data->xMin) {
                    data->xMin = x;
                }
                if (x > data->xMax) {
                    data->xMax = x;
                }

                // GET THE X RANGE FROM THE ZMAX VALUE
                x = ((float *)data->buffer)[index + 0] * data->zMax + ((float *)data->buffer)[index + 1];
                if (x < data->xMin) {
                    data->xMin = x;
                }
                if (x > data->xMax) {
                    data->xMax = x;
                }

                // GET THE X RANGE FROM THE ZMIN VALUE
                float y = ((float *)data->buffer)[index + 2] * data->zMin + ((float *)data->buffer)[index + 3];
                if (y < data->yMin) {
                    data->yMin = y;
                }
                if (y > data->yMax) {
                    data->yMax = y;
                }

                // GET THE Y RANGE FROM THE ZMAX VALUE
                y = ((float *)data->buffer)[index + 2] * data->zMax + ((float *)data->buffer)[index + 3];
                if (y < data->yMin) {
                    data->yMin = y;
                }
                if (y > data->yMax) {
                    data->yMax = y;
                }

                // INCREMENT THE INDEX TO THE NEXT PIXEL
                index += offset;
            }
        }
    } else {
        __m128 minVec = _mm_set1_ps(1e6f);
        __m128 maxVec = _mm_set1_ps(-1e6f);

        float *buffer = (float *)this->constScanLine(0);
        if (data->style == StyleXYZWRCPQLookUpTable) {
            for (unsigned int ind = 0; ind < 2 * width()*height(); ind++) {
                __m128 pixVec = _mm_load_ps(&buffer[8 * ind]);
                __m128 mskVec = _mm_cmpeq_ps(pixVec, pixVec);
                if (_mm_test_all_ones(_mm_castps_si128(mskVec))) {
                    minVec = _mm_min_ps(minVec, pixVec);
                    maxVec = _mm_max_ps(maxVec, pixVec);
                }
            }
        } else {
            for (unsigned int ind = 0; ind < width()*height(); ind++) {
                __m128 pixVec = _mm_load_ps(&buffer[4 * ind]);
                __m128 mskVec = _mm_cmpeq_ps(pixVec, pixVec);
                if (_mm_test_all_ones(_mm_castps_si128(mskVec))) {
                    minVec = _mm_min_ps(minVec, pixVec);
                    maxVec = _mm_max_ps(maxVec, pixVec);
                }
            }
        }

        *(int *)&(data->xMin) = _mm_extract_ps(minVec, 0);
        *(int *)&(data->yMin) = _mm_extract_ps(minVec, 1);
        *(int *)&(data->zMin) = _mm_extract_ps(minVec, 2);

        *(int *)&(data->xMax) = _mm_extract_ps(maxVec, 0);
        *(int *)&(data->yMax) = _mm_extract_ps(maxVec, 1);
        *(int *)&(data->zMax) = _mm_extract_ps(maxVec, 2);
    }

#ifdef USESANDBOXFIXES
    data->xMin = -1.0;
    data->xMax = +1.0;
    data->yMin = -1.0;
    data->yMax = +1.0;

    float   phiA = 180.0f / PI * atan(data->yMin / data->zMin);
    float   phiB = 180.0f / PI * atan(data->yMax / data->zMin);
    float thetaA = 180.0f / PI * atan(data->xMin / data->zMin);
    float thetaB = 180.0f / PI * atan(data->xMax / data->zMin);
#else
    float   phiA = atan(data->yMin / data->zMin);
    float   phiB = atan(data->yMax / data->zMin);
    float thetaA = atan(data->xMin / data->zMin);
    float thetaB = atan(data->xMax / data->zMin);
#endif

    data->verticalFieldOfView = fabs(phiA) + fabs(phiB);
    data->horizontalFieldOfView = fabs(thetaA) + fabs(thetaB);

    qDebug() << data->xMin << data->xMax << data->yMin << data->yMax << data->zMin << data->zMax;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULookUpTableLoader::LAULookUpTableLoader(QString filename, unsigned short drc, unsigned char *bffr, QObject *parent) : QThread(parent), tiff(NULL), buffer(bffr), step(0), rows(0), directory(drc)
{
    // OPEN INPUT TIFF FILE FROM DISK
    tiff = TIFFOpen(filename.toLatin1(), "r");
    if (tiff) {
        // SET THE DIRECTORY
        TIFFSetDirectory(tiff, directory);

        unsigned int cols = 0;
        unsigned short chns = 0;
        unsigned short bits = 0;

        // GET THE NUMBER OF ROWS, COLUMNS, AND BITS PER PIXEL
        TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &rows);
        TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &cols);
        TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &chns);
        TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bits);

        // CALCULATE THE NUMBER OF BYTES PER IMAGE ROW
        step = cols * chns * bits / 8;
    } else {
        qDebug() << "TIFFLoader Error:" << filename << directory;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAULookUpTableLoader::~LAULookUpTableLoader()
{
    if (tiff) {
        TIFFClose(tiff);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAULookUpTableLoader::run()
{
    if (buffer && tiff) {
        // ITERATE THROUGH EACH ROW READING INTO THE SUPPLIED MEMORY BUFFER
        for (unsigned int row = 0; row < rows; row++) {
            TIFFReadScanline(tiff, (buffer + row * step), (int)row);
        }
    }
}
