#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QCamera>
#include <QCameraFocus>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QThread>
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

signals:
    void doCapture();

private slots:
    void onReadyForCaptureChanged(bool ready);
    void onImageCaptured(int id, const QImage& preview);
    void onImageViewfinder(const QImage& preview);
    void on_captureButton_released();
    void on_exitButton_released();

private:
    Ui::Widget* ui;
    QThread* _cameraWorkerThread;
    CameraWorker* _cameraWorker;
    //    QCamera* _camera;
    //    QCameraFocus* _cameraFocus;
    //    QCameraImageCapture* _cameraImageCapture;
};

#endif  // WIDGET_H
