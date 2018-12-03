#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QCamera>
#include <QCameraFocus>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QVideoProbe>
#include <QElapsedTimer>
#include <atomic>

class CameraWorker : public QObject
{
    Q_OBJECT
public:
    explicit CameraWorker(QObject* parent = nullptr);

    void init();
    void capture();
    void startCamera();
    void stopCamera();
    void startViewFinder();
    void stopViewFinder();

private:
    static void imageCleanupHandler(void* info);
    int* convertYUV420_NV21toRGB8888(unsigned char data[], int width, int height);
    int convertYUVtoRGB(int y, int u, int v);

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
    QElapsedTimer* _elapsedTimer;
    std::atomic_bool _isViewFinderEnabled;
};

#endif  // CAMERAWORKER_H
