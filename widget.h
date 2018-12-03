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

protected:
    void keyPressEvent(QKeyEvent* event);

signals:
    void doCapture();
    void doStart();
    void doStop();

private slots:
    void onReadyForCaptureChanged(bool ready);
    void onImageCaptured(int id, const QImage& preview);
    void onImageViewfinder(const QImage &preview);
    void on_captureButton_released();
    void on_exitButton_released();
    void on_startButton_released();
    void on_stopButton_released();
    void onTimerTimeout();

private:
    Ui::Widget* ui;
    QThread* _cameraWorkerThread;
    CameraWorker* _cameraWorker;    
    QTimer _timer;
};

#endif  // WIDGET_H
