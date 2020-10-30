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

#ifndef LAULOOKUPTABLE_H
#define LAULOOKUPTABLE_H

#include <QtGui>
#include <QtCore>
#include <QThread>
#ifndef USEHEADLESS
#include <QFileDialog>
#include <QtOpenGL>
#endif

#include "emmintrin.h"
#include "smmintrin.h"
#include "tmmintrin.h"
#include "xmmintrin.h"

#include "laumemoryobject.h"

#ifndef PI
#define PI 3.14159265359
#endif

using namespace LAU3DVideoParameters;

namespace libtiff
{
#include "tiffio.h"
}

enum LAULookUpTableStyle  { StyleLinear, StyleFourthOrderPoly, StyleFourthOrderPolyAugmentedReality, StyleFourthOrderPolyWithPhaseUnwrap, StyleXYZPLookUpTable, StyleXYZWRCPQLookUpTable, StyleUndefined };

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAULookUpTableData : public QSharedData
{
public:
    LAULookUpTableData();
    LAULookUpTableData(const LAULookUpTableData &other);
    ~LAULookUpTableData();

    QString filename;

    QString xmlString;
    QString makeString;
    QString modelString;
    QString softwareString;

    void *buffer;
    LAULookUpTableStyle style;
    float xMin, xMax, yMin, yMax, zMin, zMax, pMin, pMax;
    float horizontalFieldOfView;
    float verticalFieldOfView;
    unsigned int numRows, numCols, numChns;
    unsigned long long numSmps;

    static int instanceCounter;

    void allocateBuffer();
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAULookUpTable
{
public:
    explicit LAULookUpTable(unsigned int cols = 0, unsigned int rows = 0, LAUVideoPlaybackDevice device = DeviceProsilicaLCG, float hFov = 0.0f, float vFov = 0.0f, float zMin = 0.0f, float zMax = 0.0f, float pMin = 0.0f, float pMax = 1.0f);
    explicit LAULookUpTable(const QString filename);
    ~LAULookUpTable();

    LAULookUpTable(libtiff::TIFF *currentTiffDirectory);
    LAULookUpTable(const LAULookUpTable &other) : data(other.data) { ; }
    LAULookUpTable &operator = (const LAULookUpTable &other)
    {
        if (this != &other) {
            data = other.data;
        }
        return (*this);
    }

    void replace(const LAULookUpTable &other);

    bool save(QString filename = QString());
    bool save(libtiff::TIFF *currentTiffDirectory);
    bool load(libtiff::TIFF *inTiff);

    bool isNull() const
    {
        return (data->buffer == nullptr);
    }

    bool isValid() const
    {
        return (!isNull());
    }

    unsigned int width() const
    {
        return (data->numCols);
    }

    unsigned int height() const
    {
        return (data->numRows);
    }

    unsigned int colors() const
    {
        return (data->numChns);
    }

    unsigned int step() const
    {
        return (data->numCols * data->numChns * sizeof(float));
    }

    LAULookUpTableStyle style() const
    {
        return (data->style);
    }

    QPoint size() const
    {
        return (QPoint((int)width(), (int)height()));
    }

    unsigned char *scanLine(unsigned int row)
    {
        return (&(((unsigned char *)(data->buffer))[row * step()]));
    }

    unsigned char *constScanLine(unsigned int row) const
    {
        return (&(((unsigned char *)(data->buffer))[row * step()]));
    }

    void setFilename(const QString string)
    {
        data->filename = string;
    }

    QString filename() const
    {
        return (data->filename);
    }

    void setXmlString(const QString string)
    {
        data->xmlString = string;
    }

    QString xmlString() const
    {
        return (data->xmlString);
    }

    void setMakeString(const QString string)
    {
        data->makeString = string;
    }

    QString makeString() const
    {
        return (data->makeString);
    }

    void setModelString(const QString string)
    {
        data->modelString = string;
    }

    QString modelString() const
    {
        return (data->modelString);
    }

    void setSoftwareString(const QString string)
    {
        data->softwareString = string;
    }

    QString softwareString() const
    {
        return (data->softwareString);
    }

    QPointF xLimits() const
    {
        return (QPointF(data->xMin, data->xMax));
    }

    QPointF yLimits() const
    {
        return (QPointF(data->yMin, data->yMax));
    }

    QPointF zLimits() const
    {
        return (QPointF(data->zMin, data->zMax));
    }

    QPointF pLimits() const
    {
        return (QPointF(data->pMin, data->pMax));
    }

    void setZLimits(QPointF point)
    {
        data->zMin = point.x();
        data->zMax = point.y();
        updateLimits();
    }

    QPointF fov() const
    {
        return (QPointF(data->horizontalFieldOfView, data->verticalFieldOfView));
    }

    LAULookUpTable crop(unsigned int x, unsigned int y, unsigned int w, unsigned int h);

private:
    QSharedDataPointer<LAULookUpTableData> data;
    void updateLimits();
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAULookUpTableLoader : public QThread
{
    Q_OBJECT

public:
    explicit LAULookUpTableLoader(QString flnm, unsigned short drc, unsigned char *bffr, QObject *parent = NULL);
    ~LAULookUpTableLoader();

protected:
    void run();

private:
    libtiff::TIFF *tiff;
    unsigned char *buffer;
    unsigned int step, rows;
    unsigned short directory;
};

#endif // LAULOOKUPTABLE_H
