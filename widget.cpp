#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    qRegisterMetaType<Qt::ApplicationState>("Qt::ApplicationState");
    _cameraWorkerThread = new QThread(this);
    _cameraWorkerThread->setObjectName("cameraWorkerThread");
    _cameraWorker = new CameraWorker();
    connect(_cameraWorkerThread, &QThread::started, _cameraWorker, &CameraWorker::init);
    connect(_cameraWorkerThread, &QThread::finished, this, &Widget::onCameraThreadFinished);
    //    _cameraWorker->init();
    connect(_cameraWorker, &CameraWorker::doImageCaptured, this, &Widget::onImageCaptured);
    connect(_cameraWorker, &CameraWorker::doReadyForCaptureChanged, this, &Widget::onReadyForCaptureChanged);
    connect(_cameraWorker, &CameraWorker::doImageViewfinder, this, &Widget::onImageViewfinder, Qt::QueuedConnection);
    connect(this, &Widget::doCapture, _cameraWorker, &CameraWorker::capture);

    _cameraWorker->moveToThread(_cameraWorkerThread);
    _cameraWorkerThread->start();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onCameraThreadFinished()
{
    qDebug() << "CAMERA THREAD IS FINISHED!";
}

void Widget::onReadyForCaptureChanged(bool ready)
{
    ui->captureButton->setEnabled(ready);
}

void Widget::onImageCaptured(int id, const QImage& preview)
{
    ui->captureLabel->setPixmap(QPixmap::fromImage(preview).scaled(ui->captureLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Widget::onImageViewfinder(const QImage& preview)
{
    ui->viewFinderLabel->setPixmap(QPixmap::fromImage(preview).scaled(ui->viewFinderLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Widget::on_captureButton_released()
{
    ui->captureButton->setDisabled(true);
    emit doCapture();
    //    _cameraImageCapture->capture();
}

void Widget::on_exitButton_released()
{
    qApp->exit();
}
