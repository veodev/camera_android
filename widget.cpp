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
    connect(_cameraWorker, &CameraWorker::doImageViewfinder, this, &Widget::onImageViewfinder);
    connect(_cameraWorker, &CameraWorker::doCameraModeChanged, this, &Widget::onCameraModeChanged);
    connect(this, &Widget::doCapture, _cameraWorker, &CameraWorker::capture);
    connect(this, &Widget::doStart, _cameraWorker, &CameraWorker::startCamera);
    connect(this, &Widget::doStop, _cameraWorker, &CameraWorker::stopCamera);
    connect(this, &Widget::doPhotoMode, _cameraWorker, &CameraWorker::photoMode);
    connect(this, &Widget::doVideoMode, _cameraWorker, &CameraWorker::videoMode);
    connect(this, &Widget::doRestart, _cameraWorker, &CameraWorker::reinit);
    connect(this, &Widget::doZoomIn, _cameraWorker, &CameraWorker::onZoomIn);
    connect(this, &Widget::doZoomOut, _cameraWorker, &CameraWorker::onZoomOut);

    setFocusPolicy(Qt::StrongFocus);

    _cameraWorker->moveToThread(_cameraWorkerThread);
    _cameraWorkerThread->start();

    connect(&_timer, &QTimer::timeout, this, &Widget::onTimerTimeout);    
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onCameraThreadFinished()
{
    qDebug() << "CAMERA THREAD IS FINISHED!";
}

void Widget::blockModesButton(bool isBlock)
{
    ui->photoButton->setDisabled(isBlock);
    ui->videoButton->setDisabled(isBlock);
}

void Widget::unblockModesButton()
{
    blockModesButton(false);
}

void Widget::backToViewFinder()
{
    _cameraWorker->startViewFinder();
    ui->stackedWidget->setCurrentIndex(0);
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
    QTimer::singleShot(2000, this, &Widget::backToViewFinder);
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

void Widget::onCameraModeChanged()
{
    blockModesButton(false);
}

void Widget::on_photoButton_released()
{
    blockModesButton(true);
    emit doPhotoMode();
    QTimer::singleShot(2000, this, &Widget::unblockModesButton);
}

void Widget::on_videoButton_released()
{
    blockModesButton(true);
    emit doVideoMode();
    QTimer::singleShot(2000, this, &Widget::unblockModesButton);
}

void Widget::on_restartButton_released()
{
    emit doRestart();
}

void Widget::on_minusButton_released()
{
    emit doZoomOut();
}

void Widget::on_plusButton_released()
{
    emit doZoomIn();
}
