#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QCamera>
#include <QCameraFocus>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QVideoProbe>

class CameraWorker : public QObject
{
    Q_OBJECT
public:
    explicit CameraWorker(QObject* parent = nullptr);

    void init();
    void capture();

signals:
    void doImageCaptured(int id, const QImage& preview);
    void doReadyForCaptureChanged(bool ready);
    void doImageViewfinder(const QImage& preview);

private slots:
    void onReadyForCaptureChanged(bool ready);
    void onImageCaptured(int id, const QImage& preview);
    void onVideoFrameProbed(const QVideoFrame& frame);

private:
    QCamera* _camera;
    QCameraFocus* _cameraFocus;
    QCameraImageCapture* _cameraImageCapture;
    QVideoProbe* _videoProbe;
};

#endif  // CAMERAWORKER_H