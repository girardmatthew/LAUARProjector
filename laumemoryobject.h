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

#ifndef LAUMEMORYOBJECT_H
#define LAUMEMORYOBJECT_H

#ifndef Q_PROCESSOR_ARM
#include "emmintrin.h"
#include "xmmintrin.h"
#include "tmmintrin.h"
#include "smmintrin.h"
#endif

#include <QTime>
#include <QtCore>
#include <QDebug>
#include <QObject>
#include <QString>
#include <QThread>
#include <QFileInfo>
#include <QSettings>
#include <QMatrix4x4>
#include <QSharedData>
#include <QSharedDataPointer>

#ifndef HEADLESS
#include <QFileDialog>
#endif

namespace libtiff
{
#include "tiffio.h"
}

namespace LAU3DVideoParameters
{
    enum LAUVideoPlaybackState  { StateLiveVideo, StateVideoPlayback };
    enum LAUVideoPlaybackDevice { DeviceUndefined, DeviceKinect, DevicePrimeSense, DeviceProsilicaLCG, DeviceProsilicaDPR, DeviceProsilicaIOS, DeviceProsilicaARG, DeviceProsilicaPST, DeviceProsilicaAST, DeviceProsilicaGRY, DeviceProsilicaRGB, DeviceXimea, DeviceIDS, DeviceRealSense, DeviceDemo };
    enum LAUVideoPlaybackColor  { ColorUndefined, ColorGray, ColorRGB, ColorRGBA, ColorXYZ, ColorXYZW, ColorXYZG, ColorXYZRGB, ColorXYZWRGBA };
    enum LAUVideoProjectorSynchronizationMode { ModeSlave, ModeMaster, ModeMono, ModeMasterHandshake, ModeHDMIFPGA };
    enum LAUVideoPatternSynchronizationScheme { SchemeFlashingSequence, SchemePatternBit, SchemeNone };
}

using namespace LAU3DVideoParameters;

#define MINNUMBEROFFRAMESAVAILABLE      40
#define MAXNUMBEROFFRAMESAVAILABLE      100

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUMemoryObjectData : public QSharedData
{
public:
    LAUMemoryObjectData();
    LAUMemoryObjectData(const LAUMemoryObjectData &other);
    LAUMemoryObjectData(unsigned int cols, unsigned int rows, unsigned int chns = 1, unsigned int byts = 1, unsigned int frms = 1);
    LAUMemoryObjectData(unsigned long long bytes);

    ~LAUMemoryObjectData();

    static int instanceCounter;

    unsigned int numRows, numCols, numChns, numFrms, numByts;
    unsigned int stepBytes, frameBytes;
    unsigned long long numBytesTotal;
    void *buffer;

    void allocateBuffer();
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUMemoryObject
{
public:
    LAUMemoryObject()
    {
        data = new LAUMemoryObjectData();
        elapsedTime = 0xffffffff;
    }
    LAUMemoryObject(unsigned int cols, unsigned int rows, unsigned int chns = 1, unsigned int byts = 1, unsigned int frms = 1)
    {
        data = new LAUMemoryObjectData(cols, rows, chns, byts, frms);
        elapsedTime = 0xffffffff;
    }
    LAUMemoryObject(unsigned long long bytes)
    {
        data = new LAUMemoryObjectData(bytes);
        elapsedTime = 0xffffffff;
    }
    LAUMemoryObject(const LAUMemoryObject &other) : data(other.data), transformMatrix(other.transformMatrix), anchorPt(other.anchorPt), elapsedTime(other.elapsedTime) { ; }
    LAUMemoryObject &operator = (const LAUMemoryObject &other)
    {
        if (this != &other) {
            data = other.data;
            transformMatrix = other.transformMatrix;
            anchorPt = other.anchorPt;
            elapsedTime = other.elapsedTime;
        }
        return (*this);
    }

    LAUMemoryObject(QImage image);
    LAUMemoryObject(QString filename);
    LAUMemoryObject(libtiff::TIFF *inTiff, int index = 0);

    bool save(QString filename = QString(), QWidget *parent = nullptr);
    bool save(libtiff::TIFF *otTiff, unsigned short index = 0);
    bool load(libtiff::TIFF *inTiff, unsigned short index = 0);

    unsigned int nonZeroPixelsCount() const;

    // SEE IF THE POINTERS ARE LOOKING AT SAME MEMORY
    bool operator == (const LAUMemoryObject &other) const
    {
        if (this == &other) {
            return (true);
        }
        return (data->buffer == other.data->buffer);
    }

    bool operator  < (const LAUMemoryObject &other) const
    {
        if (this == &other) {
            return (false);
        }
        return (elapsedTime   < other.elapsedTime);
    }

    bool operator  > (const LAUMemoryObject &other) const
    {
        if (this == &other) {
            return (false);
        }
        return (elapsedTime   > other.elapsedTime);
    }

    bool operator <= (const LAUMemoryObject &other) const
    {
        if (this == &other) {
            return (true);
        }
        return (elapsedTime  <= other.elapsedTime);
    }

    bool operator >= (const LAUMemoryObject &other) const
    {
        if (this == &other) {
            return (true);
        }
        return (elapsedTime  >= other.elapsedTime);
    }

    ~LAUMemoryObject() { ; }

    inline bool isNull() const
    {
        return (data->buffer == nullptr);
    }

    inline bool isValid() const
    {
        return (data->buffer != nullptr);
    }

    inline unsigned long long length() const
    {
        return (data->numBytesTotal);
    }

    inline QSize size() const
    {
        return (QSize(width(), height()));
    }

    inline unsigned int nugget() const
    {
        return (data->numChns * data->numByts);
    }

    inline unsigned int width() const
    {
        return (data->numCols);
    }

    inline unsigned int height() const
    {
        return (data->numRows);
    }

    inline unsigned int depth() const
    {
        return (data->numByts);
    }

    inline unsigned int colors() const
    {
        return (data->numChns);
    }

    inline unsigned int frames()  const
    {
        return (data->numFrms);
    }

    inline unsigned int step()    const
    {
        return (data->stepBytes);
    }

    inline unsigned int block()   const
    {
        return (data->frameBytes);
    }

    inline unsigned char *pointer()
    {
        return (scanLine(0));
    }

    inline unsigned char *constPointer() const
    {
        return (constScanLine(0));
    }

    inline unsigned char *scanLine(unsigned int row, unsigned int frame = 0)
    {
        return (&(((unsigned char *)(data->buffer))[frame * block() + row * step()]));
    }

    inline unsigned char *constScanLine(unsigned int row, unsigned int frame = 0) const
    {
        return (&(((unsigned char *)(data->buffer))[frame * block() + row * step()]));
    }

    inline unsigned char *frame(unsigned int frm = 0)
    {
        return (scanLine(0, frm));
    }

    inline unsigned char *constFrame(unsigned int frm = 0) const
    {
        return (constScanLine(0, frm));
    }

    inline QMatrix4x4 transform() const
    {
        return (transformMatrix);
    }

    inline unsigned int elapsed() const
    {
        return (elapsedTime);
    }

    inline QPoint anchor() const
    {
        return (anchorPt);
    }

    inline void setTransform(QMatrix4x4 mat)
    {
        transformMatrix = mat;
    }

    inline void setElapsed(unsigned int elps)
    {
        elapsedTime = elps;
    }

    inline void setAnchor(QPoint pt)
    {
        anchorPt = pt;
    }

    static LAUMemoryObject average(QList<LAUMemoryObject> objects);

protected:
    QSharedDataPointer<LAUMemoryObjectData> data;
    QMatrix4x4 transformMatrix;
    QPoint anchorPt;
    unsigned int elapsedTime;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUMemoryObjectWriter : public QThread
{
    Q_OBJECT

public:
    explicit LAUMemoryObjectWriter(QString flnm, LAUMemoryObject obj, QObject *parent = nullptr);
    ~LAUMemoryObjectWriter();

    bool isNull() const
    {
        return (!isValid());
    }

    bool isValid() const
    {
        return (tiff != nullptr);
    }

protected:
    void run();

private:
    libtiff::TIFF *tiff;
    LAUMemoryObject object;

signals:
    void emitSaveComplete();
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUMemoryObjectManager : public QObject
{
    Q_OBJECT

public:
    explicit LAUMemoryObjectManager(unsigned int cols = 0, unsigned int rows = 0, unsigned int chns = 0, unsigned int byts = 0, unsigned int frms = 0, QObject *parent = nullptr) : QObject(parent), numRows(rows), numCols(cols), numChns(chns), numByts(byts), numFrms(frms) { ; }
    ~LAUMemoryObjectManager();

public slots:
    void onGetFrame();
    void onReleaseFrame(LAUMemoryObject frame);

private:
    unsigned int numRows, numCols, numChns, numByts, numFrms;
    QList<LAUMemoryObject> framesAvailable;

signals:
    void emitFrame(LAUMemoryObject frame);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUModalityObject
{
public:
    LAUModalityObject(LAUMemoryObject dpt = LAUMemoryObject(), LAUMemoryObject clr = LAUMemoryObject(), LAUMemoryObject map = LAUMemoryObject()) : depth(dpt), color(clr), mappi(map) { ; }
    LAUModalityObject(const LAUModalityObject &other) : depth(other.depth), color(other.color), mappi(other.mappi) { ; }
    LAUModalityObject &operator = (const LAUModalityObject &other)
    {
        if (this != &other) {
            depth = other.depth;
            color = other.color;
            mappi = other.mappi;
        }
        return (*this);
    }

    bool isAnyValid()
    {
        return (depth.isValid() || color.isValid() || mappi.isValid());
    }

    LAUMemoryObject depth;
    LAUMemoryObject color;
    LAUMemoryObject mappi;
};

Q_DECLARE_METATYPE(LAUMemoryObject);

#endif // LAUMEMORYOBJECT_H
