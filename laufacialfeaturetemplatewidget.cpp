#include "laufacialfeaturetemplatewidget.h"

#include <QDebug>
#include <QPainter>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

#ifdef Q_OS_LINUX
    #define sizeInBytes byteCount
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUFacialFeatureTemplateLabel::LAUFacialFeatureTemplateLabel(QImage img, QWidget *parent) : QLabel(parent), mouseDownFlag(false)
{
    points << QPointF(0.000000, 0.199631);
    points << QPointF(0.000339, 0.324733);
    points << QPointF(0.023877, 0.450658);
    points << QPointF(0.061479, 0.572911);
    points << QPointF(0.117782, 0.691656);
    points << QPointF(0.188149, 0.806731);
    points << QPointF(0.263729, 0.905311);
    points << QPointF(0.358585, 0.983726);
    points << QPointF(0.464871, 1.000000);
    points << QPointF(0.581870, 0.974957);
    points << QPointF(0.686670, 0.899446);
    points << QPointF(0.787690, 0.798782);
    points << QPointF(0.861445, 0.680474);
    points << QPointF(0.916784, 0.557344);
    points << QPointF(0.958203, 0.433721);
    points << QPointF(0.990486, 0.305605);
    points << QPointF(1.000000, 0.164172);
    points << QPointF(0.031996, 0.079844);
    points << QPointF(0.089681, 0.023512);
    points << QPointF(0.178268, 0.014141);
    points << QPointF(0.265995, 0.029758);
    points << QPointF(0.352861, 0.070362);
    points << QPointF(0.571220, 0.069758);
    points << QPointF(0.651817, 0.022577);
    points << QPointF(0.745474, 0.000877);
    points << QPointF(0.838414, 0.000000);
    points << QPointF(0.910787, 0.056770);
    points << QPointF(0.460346, 0.186751);
    points << QPointF(0.457049, 0.282537);
    points << QPointF(0.453752, 0.378323);
    points << QPointF(0.450311, 0.478274);
    points << QPointF(0.369284, 0.537949);
    points << QPointF(0.410612, 0.551922);
    points << QPointF(0.456581, 0.566059);
    points << QPointF(0.508192, 0.551209);
    points << QPointF(0.550524, 0.536030);
    points << QPointF(0.153544, 0.192561);
    points << QPointF(0.201090, 0.160888);
    points << QPointF(0.270687, 0.163354);
    points << QPointF(0.338994, 0.203301);
    points << QPointF(0.268967, 0.213329);
    points << QPointF(0.199083, 0.219193);
    points << QPointF(0.603751, 0.204341);
    points << QPointF(0.665646, 0.160667);
    points << QPointF(0.740026, 0.159132);
    points << QPointF(0.790061, 0.190093);
    points << QPointF(0.742659, 0.217601);
    points << QPointF(0.668422, 0.214971);
    points << QPointF(0.285676, 0.672586);
    points << QPointF(0.342071, 0.653736);
    points << QPointF(0.407315, 0.647708);
    points << QPointF(0.457923, 0.662009);
    points << QPointF(0.509391, 0.651324);
    points << QPointF(0.574349, 0.653625);
    points << QPointF(0.643516, 0.668584);
    points << QPointF(0.575835, 0.745411);
    points << QPointF(0.514227, 0.780755);
    points << QPointF(0.458262, 0.787112);
    points << QPointF(0.402728, 0.780975);
    points << QPointF(0.343414, 0.749686);
    points << QPointF(0.313372, 0.677737);
    points << QPointF(0.405595, 0.697683);
    points << QPointF(0.456490, 0.703656);
    points << QPointF(0.512454, 0.697299);
    points << QPointF(0.615391, 0.675927);
    points << QPointF(0.507671, 0.701299);
    points << QPointF(0.456203, 0.711985);
    points << QPointF(0.405452, 0.701847);

    setImage(img);

    pointA = QPoint(image.width() * 0.10, image.height() * 0.10);
    pointB = QPoint(image.width() * 0.90, image.height() * 0.90);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFacialFeatureTemplateLabel::reset()
{
    setImage(QImage(":/Images/MovieIT.jpg"));
    pointA = QPoint(image.width() * 0.10, image.height() * 0.10);
    pointB = QPoint(image.width() * 0.90, image.height() * 0.90);
    update();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFacialFeatureTemplateLabel::setImage(QImage img)
{
    if (img.isNull()) {
        QMessageBox::warning(this, QString("Facial Feature Template"), QString("Cannot load valid image from file."));
    } else {
        float scaleFactorX = 640.0f / (float)img.width();
        float scaleFactorY = 480.0f / (float)img.height();
        float scaleFactor = qMin(scaleFactorX, scaleFactorY);

        image = img.scaled(img.width() * scaleFactor, img.height() * scaleFactor);
        image = image.convertToFormat(QImage::Format_ARGB32);

        this->setMinimumSize(image.size());

        update();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFacialFeatureTemplateLabel::loadSettings()
{
    QSettings settings;

    // LOAD THE TEMPLATE IMAGE FROM SETTINGS
    int cols = settings.value("LAUFacialFeatureTemplateLabel::image::width", image.width()).toInt();
    int rows = settings.value("LAUFacialFeatureTemplateLabel::image::height", image.height()).toInt();
    QImage img(cols, rows, QImage::Format_ARGB32);
    QByteArray byteArray = settings.value("LAUFacialFeatureTemplateLabel::image::pixels", QByteArray()).toByteArray();
    if (byteArray.size() == img.sizeInBytes()) {
        memcpy(img.bits(), byteArray.data(), (int)img.sizeInBytes());
        setImage(img);
    }

    QPoint ptA = settings.value("LAUFacialFeatureTemplateLabel::pointA", pointA).toPoint();
    if (ptA.x() > 0 && ptA.x() < image.width()) {
        if (ptA.y() > 0 && ptA.y() < image.height()) {
            pointA = ptA;
        }
    }

    QPoint ptB = settings.value("LAUFacialFeatureTemplateLabel::pointB", pointB).toPoint();
    if (ptB.x() > 0 && ptB.x() < image.width()) {
        if (ptB.y() > 0 && ptB.y() < image.height()) {
            pointB = ptB;
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFacialFeatureTemplateLabel::saveSettings()
{
    QSettings settings;
    settings.setValue("LAUFacialFeatureTemplateLabel::image::width", image.width());
    settings.setValue("LAUFacialFeatureTemplateLabel::image::height", image.height());

    QByteArray byteArray(image.sizeInBytes(), 0x00);
    if (byteArray.size() == image.sizeInBytes()) {
        memcpy(byteArray.data(), image.bits(), (int)image.sizeInBytes());
    }
    settings.setValue("LAUFacialFeatureTemplateLabel::image::pixels", byteArray);
    settings.setValue("LAUFacialFeatureTemplateLabel::pointA", pointA);
    settings.setValue("LAUFacialFeatureTemplateLabel::pointB", pointB);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QList<QPointF> LAUFacialFeatureTemplateLabel::features()
{
    QList<QPointF> pts;
    QVector2D trans(pointA.x(), pointA.y());
    QVector2D scale(pointB.x() - pointA.x(), pointB.y() - pointA.y());
    for (int n = 0; n < points.count(); n++) {
        QVector2D vector(points.at(n).x(), points.at(n).y());
        vector.setX(trans.x() + vector.x() * scale.x());
        vector.setY(trans.y() + vector.y() * scale.y());
        pts << QPointF(vector.x() / (float)image.width(), vector.y() / (float)image.height());
    }
    return (pts);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFacialFeatureTemplateLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseDownFlag) {
        QPoint point = event->pos();
        if (mouseDownPoint == 0) {
            pointA.setX(qMin(qMax(point.x(), 0), this->width()));
            pointA.setY(qMin(qMax(point.y(), 0), this->height()));
        } else if (mouseDownPoint == 1) {
            pointB.setX(qMin(qMax(point.x(), 0), this->width()));
            pointB.setY(qMin(qMax(point.y(), 0), this->height()));
        } else {
            QPoint delta = point - pointC;
            pointA.setX(qMin(qMax(pointA.x() + delta.x(), 0), this->width()));
            pointA.setY(qMin(qMax(pointA.y() + delta.y(), 0), this->height()));
            pointB.setX(qMin(qMax(pointB.x() + delta.x(), 0), this->width()));
            pointB.setY(qMin(qMax(pointB.y() + delta.y(), 0), this->height()));

            pointC = point;
        }
    }
    update();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFacialFeatureTemplateLabel::mousePressEvent(QMouseEvent *event)
{
    mouseDownFlag = false;
    if (event->button() == Qt::LeftButton) {
        QPoint point = event->pos();
        float distA = QVector2D(point - pointA).length();
        float distB = QVector2D(point - pointB).length();
        if (distA > 10 && distB > 10) {
            QRect rect(pointA, pointB);
            if (rect.left() < point.x() && point.x() < rect.right()) {
                if (rect.top() < point.y() && point.y() < rect.bottom()) {
                    mouseDownFlag = true;
                    mouseDownPoint = 2;
                    pointC = point;
                }
            }
        } else {
            if (distA < distB) {
                if (distA < 10) {
                    mouseDownFlag = true;
                    mouseDownPoint = 0;
                    pointA.setX(qMin(qMax(point.x(), 0), this->width()));
                    pointA.setY(qMin(qMax(point.y(), 0), this->height()));
                }
            } else {
                if (distB < 10) {
                    mouseDownFlag = true;
                    mouseDownPoint = 1;
                    pointB.setX(qMin(qMax(point.x(), 0), this->width()));
                    pointB.setY(qMin(qMax(point.y(), 0), this->height()));
                }
            }
            update();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFacialFeatureTemplateLabel::mouseReleaseEvent(QMouseEvent *)
{
    if (mouseDownFlag) {
        int lft = qMin(pointA.x(), pointB.x());
        int rgh = qMax(pointA.x(), pointB.x());
        int top = qMin(pointA.y(), pointB.y());
        int bot = qMax(pointA.y(), pointB.y());

        pointA = QPoint(lft, top);
        pointB = QPoint(rgh, bot);

        update();
    }
    mouseDownFlag = false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFacialFeatureTemplateLabel::resizeEvent(QResizeEvent *event)
{
    if (event->oldSize().width() > 0 && event->oldSize().height() > 0) {
        pointA.setX(qRound((float)pointA.x() * (float)event->size().width() / (float)event->oldSize().width()));
        pointA.setY(qRound((float)pointA.y() * (float)event->size().height() / (float)event->oldSize().height()));

        pointB.setX(qRound((float)pointB.x() * (float)event->size().width() / (float)event->oldSize().width()));
        pointB.setY(qRound((float)pointB.y() * (float)event->size().height() / (float)event->oldSize().height()));
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFacialFeatureTemplateLabel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // DRAW THE IMAGE IN THE BACKGROUND
    QTransform transform;
    transform.scale((float)width() / (float)image.width(), (float)height() / (float)image.height());
    painter.setTransform(transform);
    painter.drawImage(0, 0, image);

    // DRAW THE BOUNDING BOX
    transform.reset();
    painter.setTransform(transform);
    painter.setPen(QPen(Qt::black, 3.0f, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));

    QVector2D trans(pointA.x(), pointA.y());
    QVector2D scale(pointB.x() - pointA.x(), pointB.y() - pointA.y());
    for (int n = 0; n < points.count(); n++) {
        QVector2D vector(points.at(n).x(), points.at(n).y());
        vector.setX(trans.x() + vector.x() * scale.x());
        vector.setY(trans.y() + vector.y() * scale.y());
        painter.drawEllipse(vector.toPoint(), 3, 3);
    }

    painter.setPen(QPen(Qt::blue, 3.0f, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(QBrush(Qt::blue, Qt::NoBrush));
    painter.drawRect(QRect(pointA, pointB));

    painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
    painter.drawEllipse(pointA, 6, 6);
    painter.drawEllipse(pointB, 6, 6);

    painter.end();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUFacialFeatureTemplateWidget::LAUFacialFeatureTemplateWidget(QWidget *parent) : QWidget(parent), label(nullptr)
{
    this->setLayout(new QVBoxLayout);
    this->layout()->setContentsMargins(6, 6, 6, 6);
    label = new LAUFacialFeatureTemplateLabel(QImage(":/Images/MovieIT.jpg"));
    this->layout()->addWidget(label);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUFacialFeatureTemplateWidget::onLoadImage()
{
    QSettings settings;
    QString directory = settings.value("LAUMemoryObject::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    if (QDir().exists(directory) == false) {
        directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    QString filename = QFileDialog::getOpenFileName(this, QString("Load image from disk (*.tif, *.jpg, *.bmp, *.png)"), directory, QString("*.tif;*.jpg;*.bmp;*.png"));
    if (filename.isEmpty() == false) {
        settings.setValue("LAUMemoryObject::lastUsedDirectory", QFileInfo(filename).absolutePath());
        label->setImage(QImage(filename));
    } else {
        return;
    }
}

