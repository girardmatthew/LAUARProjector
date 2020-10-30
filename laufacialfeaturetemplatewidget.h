#ifndef LAUFACIALFEATURETEMPLATEWIDGET_H
#define LAUFACIALFEATURETEMPLATEWIDGET_H

#include <QImage>
#include <QLabel>
#include <QDialog>
#include <QWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <QDialogButtonBox>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUFacialFeatureTemplateLabel : public QLabel
{

public:
    explicit LAUFacialFeatureTemplateLabel(QImage img, QWidget *parent = nullptr);

    void setImage(QImage img);

    QList<QPointF> features();
    QImage face() const
    {
        return (image);
    }

    void loadSettings();
    void saveSettings();
    void reset();

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

private:
    QImage image;
    QList<QPointF> points;
    QPoint pointA, pointB, pointC;
    bool mouseDownFlag;
    int mouseDownPoint;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUFacialFeatureTemplateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAUFacialFeatureTemplateWidget(QWidget *parent = nullptr);

    QList<QPointF> features()
    {
        return (label->features());
    }

    QImage face() const
    {
        return (label->face());
    }

    void loadSettings()
    {
        label->loadSettings();
    }

    void saveSettings()
    {
        label->saveSettings();
    }


public slots:
    void onLoadImage();
    void onReset()
    {
        label->reset();
    }

signals:

public slots:

private:
    LAUFacialFeatureTemplateLabel *label;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUFacialFeatureTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUFacialFeatureTemplateDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("Template Layout Dialog"));

        widget = new LAUFacialFeatureTemplateWidget();
        widget->loadSettings();
        this->layout()->addWidget(widget);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok); // | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        //connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);

        QPushButton *button = new QPushButton(QString("Load"));
        connect(button, SIGNAL(clicked()), widget, SLOT(onLoadImage()));
        buttonBox->addButton(button, QDialogButtonBox::ActionRole);

        button = new QPushButton(QString("Reset"));
        connect(button, SIGNAL(clicked()), widget, SLOT(onReset()));
        buttonBox->addButton(button, QDialogButtonBox::ActionRole);
    }

    QList<QPointF> features()
    {
        return (widget->features());
    }

    QImage face() const
    {
        return (widget->face());
    }

protected:
    void accept()
    {
        widget->saveSettings();
        QDialog::accept();
    }

private:
    LAUFacialFeatureTemplateWidget *widget;
};

#endif // LAUFACIALFEATURETEMPLATEWIDGET_H
