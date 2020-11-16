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
#include "lauvideoplayerlabel.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUVideoPlayerLabel::LAUVideoPlayerLabel(VideoRecorderState state, QWidget *parent) : QLabel(parent), videoRecorderState(state)
{
    timerID = 0;
    timeStamp = 0;
    playState = false;
    sliderPosition = 0.0;
    previousPacketIndex = 0;
    buttonDownState = StateNoButtons;
    minTimeStamp = 1e6;
    maxTimeStamp = 0;
    currentTimeStamp = 0;

    this->setFixedHeight(86);
    this->setMinimumWidth(382);

    pixmapList << QPixmap(QString(":/Images/BackgroundTexture.tif")); //0
    pixmapList << QPixmap(QString(":/Images/CenterSwope.tif"));  //1
    pixmapList << QPixmap(QString(":/Images/LeftSwope.tif"));  //2
    pixmapList << QPixmap(QString(":/Images/RightSwope.tif"));  //3
    pixmapList << QPixmap(QString(":/Images/LeftLeftArrow.tif"));  //4
    pixmapList << QPixmap(QString(":/Images/LeftArrow.tif"));  //5

    if (videoRecorderState == StateVideoPlayer){
        pixmapList << QPixmap(QString(":/Images/PlayArrow.tif"));  //6
        pixmapList << QPixmap(QString(":/Images/PauseArrow.tif"));  //7
    } else {
        pixmapList << QPixmap(QString(":/Images/VideoRecordArrow.tif"));  //6
        pixmapList << QPixmap(QString(":/Images/VideoStopArrow.tif"));  //7
    }

    pixmapList << QPixmap(QString(":/Images/RightArrow.tif"));  //8
    pixmapList << QPixmap(QString(":/Images/RightRightArrow.tif"));  //9
    pixmapList << QPixmap(QString(":/Images/DownLeftLeftArrow.tif"));  //10
    pixmapList << QPixmap(QString(":/Images/DownLeftArrow.tif"));  //11

    if (videoRecorderState == StateVideoPlayer){
        pixmapList << QPixmap(QString(":/Images/DownPlayArrow.tif"));  //12
        pixmapList << QPixmap(QString(":/Images/DownPauseArrow.tif"));  //13
    } else {
        pixmapList << QPixmap(QString(":/Images/DownVideoRecordArrow.tif"));  //6
        pixmapList << QPixmap(QString(":/Images/DownVideoStopArrow.tif"));  //7
    }

    pixmapList << QPixmap(QString(":/Images/DownRightArrow.tif"));  //14
    pixmapList << QPixmap(QString(":/Images/DownRightRightArrow.tif"));  //15
    pixmapList << QPixmap(QString(":/Images/ProgressBarLeft.tif"));  //16
    pixmapList << QPixmap(QString(":/Images/ProgressBarCenter.tif"));  //17
    pixmapList << QPixmap(QString(":/Images/ProgressBarRight.tif"));  //18
    pixmapList << QPixmap(QString(":/Images/DownProgressBarCenter.tif"));  //19
    pixmapList << QPixmap(QString(":/Images/Decimal0.tif"));  //20
    pixmapList << QPixmap(QString(":/Images/Decimal1.tif"));  //21
    pixmapList << QPixmap(QString(":/Images/Decimal2.tif"));  //22
    pixmapList << QPixmap(QString(":/Images/Decimal3.tif"));  //23
    pixmapList << QPixmap(QString(":/Images/Decimal4.tif"));  //24
    pixmapList << QPixmap(QString(":/Images/Decimal5.tif"));  //25
    pixmapList << QPixmap(QString(":/Images/Decimal6.tif"));  //26
    pixmapList << QPixmap(QString(":/Images/Decimal7.tif"));  //27
    pixmapList << QPixmap(QString(":/Images/Decimal8.tif"));  //28
    pixmapList << QPixmap(QString(":/Images/Decimal9.tif"));  //29
    pixmapList << QPixmap(QString(":/Images/DecimalColon.tif"));  //30
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUVideoPlayerLabel::~LAUVideoPlayerLabel()
{
    qDebug() << QString("LAUVideoPlayerLabel::~LAUVideoPlayerLabel()");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::onPlayButtonClicked(bool state)
{
    if (playState != state) {
        togglePlayback();
        update();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::onLeftLeftButtonClicked()
{
    if (packetList.count()>0){
        newPacketIndex = 0;
        onUpdateSliderPosition( (double)(newPacketIndex) / (double)(packetList.count()-1) );
        buttonDownState = StateNoButtons;
        update();

        emit leftLeftButtonClicked();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::onLeftButtonClicked()
{
    if (packetList.count()>0){
        if ((--newPacketIndex) < 0) newPacketIndex = 0;
        onUpdateSliderPosition( (double)(newPacketIndex) / (double)(packetList.count()-1) );
        buttonDownState = StateNoButtons;
        update();

        emit leftButtonClicked();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::onRightButtonClicked()
{
    if (packetList.count()>0){
        if ((++newPacketIndex) >= packetList.count()) newPacketIndex = packetList.count()-1;
        onUpdateSliderPosition( (double)newPacketIndex / (double)(packetList.count()-1) );
        buttonDownState = StateNoButtons;
        update();

        emit rightButtonClicked();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::onRightRightButtonClicked()
{
    if (packetList.count()>0){
        newPacketIndex = packetList.count()-1;
        onUpdateSliderPosition( (double)newPacketIndex / (double)(packetList.count()-1) );
        buttonDownState = StateNoButtons;
        update();

        emit rightRightButtonClicked();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::onRemovePacket(LAUMemoryObject packet)
{
    // SEARCH THROUGH PACKET LIST LOOKING FOR PACKET WITH THE SAME BUFFER AS INCOMING
    for (int n=0; n<packetList.count(); n++){
        if (packetList.at(n) == packet){
            // DELETE THE PACKET FROM OUR LIST
            packetList.takeAt(n);
            return;
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::onInsertPacket(LAUMemoryObject packet)
{
    // STORE THE INCOMING POINTER AND TIME STAMP
    // INTO OUR PACKET LIST FOR PLAYING BACK LATER
    for (int n=packetList.count()-1; n >= 0; n--){
        if (packetList.at(n) < packet){
            packetList.insert(n+1, packet);

            minTimeStamp = packetList.first().elapsed();
            maxTimeStamp = packetList.last().elapsed();

            return;
        }
    }
    packetList.insert(0, packet);

    minTimeStamp = packetList.first().elapsed();
    maxTimeStamp = packetList.last().elapsed();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::onUpdateSliderPosition(double lambda)
{
    if (lambda < 0.0f){
        sliderPosition = 0.0f;
    } else if (lambda > 1.0f){
        sliderPosition = 1.0f;
    } else {
        sliderPosition = lambda;
    }

    // FIND OUT WHICH BUFFER TO EMIT BASED ON TIME STAMP
    if (packetList.count()>0){
        newPacketIndex = qRound(sliderPosition*(packetList.count()-1));
        qDebug() << QString("Emitting packet %1").arg(newPacketIndex);
        emitPacket(packetList.at(newPacketIndex));
    }
    update();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::onUpdateTimeStamp(unsigned int mSec)
{
    timeStamp = mSec;
    update();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    newPacketIndex = previousPacketIndex;

    // INCREMENT THE CURRENT TIME REGISTER TO ACCOUNT FOR 5 MILLISECOND TIMER PERIOD
    currentTimeStamp += 5;
    if (currentTimeStamp >= maxTimeStamp){
        // WE'VE REACHED THE END OF THE LINE, SO DISABLE PLAYBACK
        if (packetList.count()>0){
            newPacketIndex = packetList.count()-1;
        }
        togglePlayback();
    } else {
        // DERIVE THE CURRENT TIME SINCE LAST CALL TO THIS EVENT AND UPDATE SLIDER
        while ((int)packetList.at(newPacketIndex).elapsed() > (int)currentTimeStamp){
            newPacketIndex--;
        }
        while ((int)packetList.at(newPacketIndex).elapsed() < (int)currentTimeStamp){
            newPacketIndex++;
        }
        newPacketIndex--;
    }

    // THE INDEX HAS CHANGED, SO UPDATE THE SLIDER
    if (newPacketIndex != previousPacketIndex){
        // SAVE THE NEW PACKET INDEX FOR THE NEXT ITERATION
        previousPacketIndex = newPacketIndex;

        // UPDATE THE SLIDER AS A PERCENTAGE OF DISTANCE THROUGH PACKET LIST
        if (packetList.count()>0){
            onUpdateSliderPosition( (double)newPacketIndex / (double)(packetList.count()-1) );
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::togglePlayback()
{
    playState = !playState;
    emit playButtonClicked(playState);
    if (videoRecorderState == StateVideoPlayer){
        if (playState){
            if (packetList.count()>0){
                timerID = startTimer(5, Qt::CoarseTimer);
                if (sliderPosition == 1.0) sliderPosition = 0.0;
                currentTimeStamp = (maxTimeStamp - minTimeStamp)*sliderPosition + minTimeStamp;
            } else {
                // WITHOUT ANY DATA, WE CAN'T START PLAYING
                // SO TOGGLE PLAYBACK BACK TO THE OFF STATE
                togglePlayback();
            }
        } else {
            if (timerID) killTimer(timerID);
        }
        qDebug() << QString("LAUVideoPlayerLabel::togglePlayback()") << playState;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if (videoRecorderState == StateVideoPlayer){
        if (packetList.count() > 0){
            emit emitPacket(packetList.at(0));
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    int wdth = width();
    int hght = height();

    QPoint pos = event->pos();

    int x = pos.x();
    int y = pos.y();

    buttonDownState = StateNoButtons;

    if ((x > 68) && (x < wdth-20) && (y > hght-79) && (y < hght-55)){
        // VIDEO RECORDED DOESN'T ALLOW USER TO CHANGE POSITION OF SLIDER
        if (videoRecorderState == StateVideoPlayer){
            if (playState){
                togglePlayback();
            }
            onUpdateSliderPosition((double)(event->pos().x()-68)/(double)(wdth-88));
            buttonDownState = StateSliderButton;
        }
    } else if ((y > hght-52) && (y < hght-6)){
        if ((x > wdth/2-98) && (x < wdth/2+98)){
            if (x < wdth/2-63){
                if (videoRecorderState == StateVideoPlayer){
                    buttonDownState = StateLeftLeftButton;
                }
            } else if (x < wdth/2-22) {
                if (videoRecorderState == StateVideoPlayer){
                    buttonDownState = StateLeftButton;
                }
            } else if (x < wdth/2+22) {
                buttonDownState = StatePlayButton;
            } else if (x < wdth/2+63) {
                if (videoRecorderState == StateVideoPlayer){
                    buttonDownState = StateRightButton;
                }
            } else {
                if (videoRecorderState == StateVideoPlayer){
                    buttonDownState = StateRightRightButton;
                }
            }
        }
    }
    update();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (buttonDownState == StateNoButtons) return;

    int wdth = width();
    int hght = height();

    QPoint pos = event->pos();

    if (buttonDownState == StateSliderButton){
        onUpdateSliderPosition((double)(event->pos().x()-68)/(double)(wdth-88));
    } else {
        int x = pos.x();
        int y = pos.y();

        if ((y < hght-52) || (y > hght-6) || (x < wdth/2-98) || (x > wdth/2+98)){
            buttonDownState = StateNoButtons;
        } else if (buttonDownState == StateLeftLeftButton){
            if (x > wdth/2-63){
                buttonDownState = StateNoButtons;
            }
        } else if (buttonDownState == StateLeftButton){
            if ((x < wdth/2-63) || (x > wdth/2-22)) {
                buttonDownState = StateNoButtons;
            }
        } else if (buttonDownState == StatePlayButton){
            if ((x < wdth/2-22) || (x > wdth/2+22)) {
                buttonDownState = StateNoButtons;
            }
        } else if (buttonDownState == StateRightButton){
            if ((x < wdth/2+22) || (x > wdth/2+63)) {
                buttonDownState = StateNoButtons;
            }
        } else if (buttonDownState == StateRightRightButton){
            if (x < wdth/2+63){
                buttonDownState = StateNoButtons;
            }
        }
    }
    update();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::mouseReleaseEvent(QMouseEvent*)
{
    switch (buttonDownState){
    case (StatePlayButton):
        togglePlayback();
        break;
    case (StateLeftLeftButton):
        if (playState) togglePlayback();
        onLeftLeftButtonClicked();
        break;
    case (StateLeftButton):
        if (playState) togglePlayback();
        onLeftButtonClicked();
        break;
    case (StateRightButton):
        if (playState) togglePlayback();
        onRightButtonClicked();
        break;
    case (StateRightRightButton):
        if (playState) togglePlayback();
        onRightRightButtonClicked();
        break;
    default:
        break;
    }
    buttonDownState = StateNoButtons;
    update();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUVideoPlayerLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    int wdth = width();
    int hght = height();

    QPainter painter(this);

    painter.drawTiledPixmap(0, 0, wdth, hght, pixmapList.at(0));

    if (videoRecorderState == StateVideoPlayer){
        if (buttonDownState != StateLeftLeftButton){
            painter.drawPixmap(wdth/2-98, hght-52, 35, 46, pixmapList.at(4));
        } else {
            painter.drawPixmap(wdth/2-98, hght-52, 35, 46, pixmapList.at(10));
        }

        if (buttonDownState != StateLeftButton){
            painter.drawPixmap(wdth/2-63, hght-52, 41, 46, pixmapList.at(5));
        } else {
            painter.drawPixmap(wdth/2-63, hght-52, 41, 46, pixmapList.at(11));
        }

        if (buttonDownState != StateRightButton){
            painter.drawPixmap(wdth/2+22, hght-52, 41, 46, pixmapList.at(8));
        } else {
            painter.drawPixmap(wdth/2+22, hght-52, 41, 46, pixmapList.at(14));
        }

        if (buttonDownState != StateRightRightButton){
            painter.drawPixmap(wdth/2+63, hght-52, 35, 46, pixmapList.at(9));
        } else {
            painter.drawPixmap(wdth/2+63, hght-52, 35, 46, pixmapList.at(15));
        }
    }

    if (buttonDownState != StatePlayButton){
        if (playState){
            painter.drawPixmap(wdth/2-22, hght-52, 45, 46, pixmapList.at(7));
        } else {
            painter.drawPixmap(wdth/2-22, hght-52, 45, 46, pixmapList.at(6));
        }
    } else {
        if (playState){
            painter.drawPixmap(wdth/2-22, hght-52, 45, 46, pixmapList.at(13));
        } else {
            painter.drawPixmap(wdth/2-22, hght-52, 45, 46, pixmapList.at(12));
        }
    }

    // DRAW THE SWOPE PATTERN ALONG BOTTOM OF WINDOW
    painter.drawTiledPixmap(191, hght-39, wdth-382, 39, pixmapList.at(1));
    painter.drawPixmap(0, hght-39, 191, 39, pixmapList.at(2));
    painter.drawPixmap(wdth-191, hght-39, 191, 39, pixmapList.at(3));

    // DRAW PROGRESS BAR
    painter.drawTiledPixmap(68, hght-79, wdth-87, 24, pixmapList.at(17));
    painter.drawTiledPixmap(68, hght-79, (int)((wdth-87)*sliderPosition), 24, pixmapList.at(19));
    painter.drawPixmap(7, hght-79, 61, 24, pixmapList.at(16));
    painter.drawPixmap(wdth-19, hght-79, 12, 24, pixmapList.at(18));

    // DRAW TIME ON PROGRESS BAR
    int localTimeStamp = 0;
    if (videoRecorderState == StateVideoPlayer){
        if (minTimeStamp < maxTimeStamp){
            localTimeStamp = (maxTimeStamp-minTimeStamp)*sliderPosition + minTimeStamp;
        }
    } else {
        localTimeStamp = timeStamp;
    }
    int minutes = localTimeStamp/60000;
    int seconds = (localTimeStamp - minutes*60000)/1000;
    int milliseconds = localTimeStamp - minutes*60000 - seconds*1000;

    painter.drawPixmap(45, hght-72, 6, 9, pixmapList.at(20+(milliseconds/100)%10));
    painter.drawPixmap(51, hght-72, 6, 9, pixmapList.at(20+(milliseconds/10 )%10));
    painter.drawPixmap(43, hght-72, 2, 9, pixmapList.at(30));

    painter.drawPixmap(31, hght-72, 6, 9, pixmapList.at(20+(seconds/10)%10));
    painter.drawPixmap(37, hght-72, 6, 9, pixmapList.at(20+(seconds   )%10));
    painter.drawPixmap(29, hght-72, 2, 9, pixmapList.at(30));

    painter.drawPixmap(17, hght-72, 6, 9, pixmapList.at(20+(minutes/10)%10));
    painter.drawPixmap(23, hght-72, 6, 9, pixmapList.at(20+(minutes   )%10));

    painter.end();
}
