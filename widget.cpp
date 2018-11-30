#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    _camera = new QCamera(QCamera::BackFace);
    _camera->setCaptureMode(QCamera::CaptureStillImage);

    _cameraFocus = _camera->focus();
    _cameraFocus->setFocusMode(QCameraFocus::AutoFocus);

    QCameraViewfinder* viewFinder = new QCameraViewfinder;
    _camera->setViewfinder(viewFinder);

    _cameraImageCapture = new QCameraImageCapture(_camera);
    connect(_cameraImageCapture, &QCameraImageCapture::readyForCaptureChanged, this, &Widget::onReadyForCaptureChanged);
    connect(_cameraImageCapture, &QCameraImageCapture::imageCaptured, this, &Widget::onImageCaptured);

    _camera->start();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onReadyForCaptureChanged(bool ready)
{
    ui->captureButton->setEnabled(ready);
}

void Widget::onImageCaptured(int id, const QImage& preview)
{
    ui->captureLabel->setPixmap(QPixmap::fromImage(preview).scaled(ui->captureLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Widget::on_captureButton_released()
{
    ui->captureButton->setDisabled(true);
    _cameraImageCapture->capture();
}
