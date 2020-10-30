#ifndef LAUKINECTGLFILTER_H
#define LAUKINECTGLFILTER_H

#include "laulookuptable.h"
#include "lauabstractglfilter.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUKinectGLFilter : public LAUAbstractGLFilter
{
    Q_OBJECT

public:
    explicit LAUKinectGLFilter(unsigned int dCols, unsigned int dRows, unsigned int cCols, unsigned int cRows, QObject *parent = nullptr);
    ~LAUKinectGLFilter();

    void initialize();

    QOpenGLFramebufferObject *depthFBO() const
    {
        return (frameBufferObjectB);
    }

    QOpenGLFramebufferObject *scanFBO() const
    {
        return (frameBufferObjectD);
    }

    QOpenGLFramebufferObject *faceFBO() const
    {
        return (frameBufferObjectA);
    }

    QOpenGLFramebufferObject *videoFBO() const
    {
        return (frameBufferObjectC);
    }

    void scanObject(float *buffer);
    void setFace(const unsigned short *buffer);
    void setDepth(const unsigned short *buffer);
    void setVideo(const unsigned char *buffer);
    void setLookUpTable(LAULookUpTable lut);

private:
    unsigned int rgbCols, rgbRows;
    LAUMemoryObject objectA, objectB, objectC, objectD;

    LAULookUpTable lookUpTable;
    QOpenGLShaderProgram programA, programB, programC, programD;
    QOpenGLFramebufferObject *frameBufferObjectA, *frameBufferObjectB;
    QOpenGLFramebufferObject *frameBufferObjectC, *frameBufferObjectD;
    QOpenGLTexture *faceTexture, *depthTexture, *videoTexture;
    QOpenGLTexture *textureLookUpTable;
};

#endif // LAUKINECTGLFILTER_H
