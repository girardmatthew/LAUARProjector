#ifndef LAUREALSENSEGLFILTER_H
#define LAUREALSENSEGLFILTER_H

#include "laulookuptable.h"
#include "lauabstractglfilter.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURealSenseGLFilter : public LAUAbstractGLFilter
{
    Q_OBJECT

public:
    explicit LAURealSenseGLFilter(unsigned int cols, unsigned int rows, QObject *parent = nullptr);
    ~LAURealSenseGLFilter();

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
    void setFace(const unsigned char *buffer);
    void setDepth(const unsigned short *buffer);
    void setVideo(const unsigned char *buffer);
    void setLookUpTable(LAULookUpTable lut);

private:
    //LAUMemoryObject objectA, objectB, objectC, objectD;

    LAULookUpTable lookUpTable;
    QOpenGLShaderProgram programA, programB, programC;
    QOpenGLFramebufferObject *frameBufferObjectA, *frameBufferObjectB;
    QOpenGLFramebufferObject *frameBufferObjectC, *frameBufferObjectD;
    QOpenGLTexture *faceTexture, *depthTexture, *videoTexture;
    QOpenGLTexture *textureLookUpTable;
};

#endif // LAUREALSENSEGLFILTER_H
