#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QCamera>
#include <QCameraFocus>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QThread>
#include <QTimer>
#include "cameraworker.h"

namespace Ui
{
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = nullptr);
    ~Widget();

    void onCameraThreadFinished();
    void blockModesButton(bool isBlock);
    void unblockModesButton();
    void backToViewFinder();

protected:
    void keyPressEvent(QKeyEvent* event);

signals:
    void doCapture();
    void doStart();
    void doStop();
    void doPhotoMode();
    void doVideoMode();
    void doRestart();
    void doZoomIn();
    void doZoomOut();

private slots:
    void onReadyForCaptureChanged(bool ready);
    void onImageCaptured(int id, const QImage& preview);
    void onImageViewfinder(const QImage &preview);
    void on_captureButton_released();
    void on_exitButton_released();
    void on_startButton_released();
    void on_stopButton_released();
    void onTimerTimeout();
    void onCameraModeChanged();

    void on_photoButton_released();

    void on_videoButton_released();

    void on_restartButton_released();

    void on_minusButton_released();

    void on_plusButton_released();

private:
    Ui::Widget* ui;
    QThread* _cameraWorkerThread;
    CameraWorker* _cameraWorker;    
    QTimer _timer;
    QCamera::CaptureMode _currentMode;
};

#endif  // WIDGET_H
