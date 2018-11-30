#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QCamera>
#include <QCameraFocus>
#include <QCameraImageCapture>
#include <QCameraViewfinder>

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

private slots:
    void onReadyForCaptureChanged(bool ready);
    void onImageCaptured(int id, const QImage& preview);

    void on_captureButton_released();

private:
    Ui::Widget* ui;

    QCamera* _camera;
    QCameraFocus* _cameraFocus;
    QCameraImageCapture* _cameraImageCapture;
};

#endif  // WIDGET_H
