#ifndef LAUBASLERGLFILTER_H
#define LAUBASLERGLFILTER_H

#include "laulookuptable.h"
#include "lauabstractglfilter.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUBaslerGLFilter : public LAUAbstractGLFilter
{
    Q_OBJECT

public:
    explicit LAUBaslerGLFilter(unsigned int cols, unsigned int rows, QObject *parent = nullptr);
    ~LAUBaslerGLFilter();

    void getFrame();
    void initialize();
    void updateBuffer();

    QOpenGLFramebufferObject *scanFBO() const
    {
        return (frameBufferObjectE);
    }

    QOpenGLFramebufferObject *faceFBO() const
    {
        return (frameBufferObjectH);
    }

    LAUMemoryObject scanObj();

    void setLookUpTable(LAULookUpTable lut);
    void setFace(unsigned short *buffer);
    void setFrame(unsigned short *buffer, int frm);

public slots:
    void onSetMTNThreshold(unsigned int val)
    {
        mtnThreshold = val;
    }

    void onSetSNRThreshold(unsigned int val)
    {
        snrThreshold = val;
    }

private:
    unsigned int snrThreshold, mtnThreshold;

    QOpenGLTexture *frameTexture, *textureMin, *textureMax, *textureLookUpTable;
    QOpenGLShaderProgram programA, programB, programC, programD, programE, programF, programG;
    QOpenGLFramebufferObject *frameBufferObjectA, *frameBufferObjectB, *frameBufferObjectC;
    QOpenGLFramebufferObject *frameBufferObjectD, *frameBufferObjectE, *frameBufferObjectF;
    QOpenGLFramebufferObject *frameBufferObjectG, *frameBufferObjectH;

    LAUMemoryObject localObject;
    LAULookUpTable lookUpTable;
};

#endif // LAUBASLERGLFILTER_H
