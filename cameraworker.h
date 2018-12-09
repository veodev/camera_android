#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QCamera>
#include <QCameraFocus>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QCameraExposure>
#include <QMediaRecorder>
#include <QVideoProbe>
#include <atomic>
#include <QElapsedTimer>

class CameraWorker : public QObject
{
    Q_OBJECT
public:
    explicit CameraWorker(QObject* parent = nullptr);

    void init();
    void reinit();
    void capture();
    void startCamera();
    void stopCamera();
    void startViewFinder();
    void stopViewFinder();
    void photoMode();
    void videoMode();

public slots:
    void onZoomIn();
    void onZoomOut();
    void onRecord();
    void onStop();
    void onPhotoMode();
    void onVideoMode();
    void onReload();

private:
    void checkSupportedImageSettings();
    void checkSupportedVideoSettings();
    void checkSupportedViewfinderResolutions();

signals:
    void doImageCaptured(int id, const QImage& preview);
    void doReadyForCaptureChanged(bool ready);
    void doImageViewfinder(const QImage& preview);
    void doVideoDurationChanged(qint64 duration);
    void doDigitalZoomChanged(qreal value);
    void doCaptureModeChanged(QCamera::CaptureModes mode);
    void doMediaRecorderStateChanged(QMediaRecorder::State state);
    void doImageSaved(QString& fileName);
    void doVideoSaved(QString& fileName);
    void doCameraModeChanged();

private slots:
    void onReadyForCaptureChanged(bool ready);
    void onImageCaptured(int id, const QImage& preview);
    void onVideoFrameProbed(const QVideoFrame& frame);
    void onImageSaved(int id, const QString& fileName);
    void onMediaRecorderError(QMediaRecorder::Error error);
    void onCameraStatusChanged(QCamera::Status status);
    void onCameraStateChanged(QCamera::State state);
    void onCameraLockStatusChanged(QCamera::LockType lock, QCamera::LockStatus status, QCamera::LockChangeReason reason);
    void onCameraError(QCamera::Error);
    void onCaptureModeChanged(QCamera::CaptureModes mode);

private:
    QCamera* _camera;
    QCameraFocus* _cameraFocus;
    QCameraImageCapture* _cameraImageCapture;
    QVideoProbe* _videoProbe;
    QMediaRecorder* _mediaRecorder;
    QCameraExposure* _cameraExposure;
    std::atomic_bool _isViewFinderEnabled;
    QString _videoFileName;
    QElapsedTimer _elapsedTimer;
    QCamera::CaptureMode _currentMode;
    QCameraViewfinder* _viewFinder;
    bool _isChangingMode;
};

#endif  // CAMERAWORKER_H
