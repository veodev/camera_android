#include "widget.h"
#include "ui_widget.h"
#include <QKeyEvent>
#include <QDebug>

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
    connect(_cameraWorker, &CameraWorker::doImageCaptured, this, &Widget::onImageCaptured);
    connect(_cameraWorker, &CameraWorker::doReadyForCaptureChanged, this, &Widget::onReadyForCaptureChanged);
    connect(_cameraWorker, &CameraWorker::doImageViewfinder, this, &Widget::onImageViewfinder, Qt::QueuedConnection);
    connect(this, &Widget::doCapture, _cameraWorker, &CameraWorker::capture);
    connect(this, &Widget::doStart, _cameraWorker, &CameraWorker::startCamera);
    connect(this, &Widget::doStop, _cameraWorker, &CameraWorker::stopCamera);

    setFocusPolicy(Qt::StrongFocus);

    _cameraWorker->moveToThread(_cameraWorkerThread);
    _cameraWorkerThread->start();

    connect(&_timer, &QTimer::timeout, this, &Widget::onTimerTimeout);
    _timer.start(1000);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onCameraThreadFinished()
{
    qDebug() << "CAMERA THREAD IS FINISHED!";
}

void Widget::keyPressEvent(QKeyEvent *ke)
{    
    qDebug() << ke->key();
    if (ke->key() == Qt::Key_Return) {
        on_captureButton_released();
    }
}

void Widget::onReadyForCaptureChanged(bool ready)
{
    ui->captureButton->setEnabled(ready);
}

void Widget::onImageCaptured(int id, const QImage& preview)
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->captureLabel->setPixmap(QPixmap::fromImage(preview).scaled(ui->captureLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    _cameraWorker->startViewFinder();    
}

void Widget::onImageViewfinder(const QImage& preview)
{        
    ui->viewFinderLabel->setPixmap(QPixmap::fromImage(preview).scaled(ui->viewFinderLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Widget::on_captureButton_released()
{
    if (ui->stackedWidget->currentIndex() == 1) {
        ui->stackedWidget->setCurrentIndex(0);
    }
    else {
        _cameraWorker->stopViewFinder();
        ui->captureButton->setDisabled(true);
        emit doCapture();
    }
}

void Widget::on_exitButton_released()
{
    qApp->exit();
}

void Widget::on_startButton_released()
{
    emit doStart();
}

void Widget::on_stopButton_released()
{
    emit doStop();
}

void Widget::onTimerTimeout()
{
    qDebug() << this->hasFocus();
}
