#include <private/qvideoframe_p.h>
#include <QKeyEvent>
#include <QVideoFrame>
#include <QDateTime>
#include <QThread>

#include "cameraworker.h"
#include "accessories.h"

CameraWorker::CameraWorker(QObject* parent)
    : QObject(parent)
    , _isChangingMode(false)
{
}

void CameraWorker::init()
{
    _isViewFinderEnabled = true;
    _camera = new QCamera(QCamera::BackFace);
    connect(_camera, &QCamera::captureModeChanged, this, &CameraWorker::onCaptureModeChanged);
    connect(_camera, &QCamera::captureModeChanged, this, &CameraWorker::doCaptureModeChanged);
    connect(_camera, &QCamera::statusChanged, this, &CameraWorker::onCameraStatusChanged);
    connect(_camera, &QCamera::stateChanged, this, &CameraWorker::onCameraStateChanged);
    connect(_camera, QOverload<QCamera::LockType, QCamera::LockStatus, QCamera::LockChangeReason>::of(&QCamera::lockStatusChanged), this, &CameraWorker::onCameraLockStatusChanged);
    connect(_camera, QOverload<QCamera::Error>::of(&QCamera::error), this, &CameraWorker::onCameraError);
    _camera->setCaptureMode(QCamera::CaptureStillImage);
    _currentMode = QCamera::CaptureStillImage;

    checkSupportedViewfinderResolutions();
    _cameraExposure = _camera->exposure();
    _cameraExposure->setExposureMode(QCameraExposure::ExposureAuto);
    _cameraExposure->setFlashMode(QCameraExposure::FlashFill);

    QCameraViewfinderSettings settings;
    settings.setResolution(352, 288);
    settings.setPixelAspectRatio(352, 288);
    settings.setPixelFormat(QVideoFrame::Format_ARGB32_Premultiplied);
    _camera->setViewfinderSettings(settings);

    _cameraFocus = _camera->focus();
    _cameraFocus->setFocusMode(QCameraFocus::ContinuousFocus);
    connect(_cameraFocus, &QCameraFocus::digitalZoomChanged, this, &CameraWorker::doDigitalZoomChanged);

    _viewFinder = new QCameraViewfinder;
    _camera->setViewfinder(_viewFinder);
    _cameraImageCapture = new QCameraImageCapture(_camera);
    checkSupportedImageSettings();
    _cameraImageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);
    connect(_cameraImageCapture, &QCameraImageCapture::readyForCaptureChanged, this, &CameraWorker::onReadyForCaptureChanged);
    connect(_cameraImageCapture, &QCameraImageCapture::imageCaptured, this, &CameraWorker::onImageCaptured);
    connect(_cameraImageCapture, &QCameraImageCapture::imageSaved, this, &CameraWorker::onImageSaved);

    _videoProbe = new QVideoProbe;
    _videoProbe->setSource(_camera);
    connect(_videoProbe, &QVideoProbe::videoFrameProbed, this, &CameraWorker::onVideoFrameProbed);

    QVideoEncoderSettings videoSettings;
    videoSettings.setCodec("h264");
    videoSettings.setResolution(352, 288);
    videoSettings.setQuality(QMultimedia::NormalQuality);
    videoSettings.setFrameRate(30);
    videoSettings.setBitRate(10);

    _mediaRecorder = new QMediaRecorder(_camera);
    checkSupportedVideoSettings();
    _mediaRecorder->setVideoSettings(videoSettings);
    _mediaRecorder->setContainerFormat("mp4");
    connect(_mediaRecorder, &QMediaRecorder::durationChanged, this, &CameraWorker::doVideoDurationChanged);
    connect(_mediaRecorder, QOverload<QMediaRecorder::Error>::of(&QMediaRecorder::error), this, &CameraWorker::onMediaRecorderError);
    connect(_mediaRecorder, &QMediaRecorder::stateChanged, this, &CameraWorker::doMediaRecorderStateChanged);
}

void CameraWorker::reinit()
{
    _isViewFinderEnabled = true;
    disconnect(_camera, &QCamera::captureModeChanged, this, &CameraWorker::onCaptureModeChanged);
    disconnect(_camera, &QCamera::captureModeChanged, this, &CameraWorker::doCaptureModeChanged);
    disconnect(_camera, &QCamera::statusChanged, this, &CameraWorker::onCameraStatusChanged);
    disconnect(_camera, &QCamera::stateChanged, this, &CameraWorker::onCameraStateChanged);
    disconnect(_camera, QOverload<QCamera::LockType, QCamera::LockStatus, QCamera::LockChangeReason>::of(&QCamera::lockStatusChanged), this, &CameraWorker::onCameraLockStatusChanged);
    disconnect(_camera, QOverload<QCamera::Error>::of(&QCamera::error), this, &CameraWorker::onCameraError);
    delete _camera;
    _camera = new QCamera(QCamera::BackFace);
    connect(_camera, &QCamera::captureModeChanged, this, &CameraWorker::onCaptureModeChanged);
    connect(_camera, &QCamera::captureModeChanged, this, &CameraWorker::doCaptureModeChanged);
    connect(_camera, &QCamera::statusChanged, this, &CameraWorker::onCameraStatusChanged);
    connect(_camera, &QCamera::stateChanged, this, &CameraWorker::onCameraStateChanged);
    connect(_camera, QOverload<QCamera::LockType, QCamera::LockStatus, QCamera::LockChangeReason>::of(&QCamera::lockStatusChanged), this, &CameraWorker::onCameraLockStatusChanged);
    connect(_camera, QOverload<QCamera::Error>::of(&QCamera::error), this, &CameraWorker::onCameraError);
    _camera->setCaptureMode(QCamera::CaptureStillImage);
    _currentMode = QCamera::CaptureStillImage;

    _cameraExposure = _camera->exposure();
    _cameraExposure->setExposureMode(QCameraExposure::ExposureAuto);
    _cameraExposure->setFlashMode(QCameraExposure::FlashFill);

    QCameraViewfinderSettings settings;
    settings.setResolution(352, 288);
    settings.setPixelAspectRatio(352, 288);
    settings.setPixelFormat(QVideoFrame::Format_ARGB32_Premultiplied);
    _camera->setViewfinderSettings(settings);

    _cameraFocus = _camera->focus();
    _cameraFocus->setFocusMode(QCameraFocus::ContinuousFocus);

    delete _viewFinder;
    _viewFinder = new QCameraViewfinder;
    _camera->setViewfinder(_viewFinder);
    disconnect(_cameraImageCapture, &QCameraImageCapture::readyForCaptureChanged, this, &CameraWorker::onReadyForCaptureChanged);
    disconnect(_cameraImageCapture, &QCameraImageCapture::imageCaptured, this, &CameraWorker::onImageCaptured);
    disconnect(_cameraImageCapture, &QCameraImageCapture::imageSaved, this, &CameraWorker::onImageSaved);
    delete _cameraImageCapture;
    _cameraImageCapture = new QCameraImageCapture(_camera);
    _cameraImageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);
    connect(_cameraImageCapture, &QCameraImageCapture::readyForCaptureChanged, this, &CameraWorker::onReadyForCaptureChanged);
    connect(_cameraImageCapture, &QCameraImageCapture::imageCaptured, this, &CameraWorker::onImageCaptured);
    connect(_cameraImageCapture, &QCameraImageCapture::imageSaved, this, &CameraWorker::onImageSaved);

    disconnect(_videoProbe, &QVideoProbe::videoFrameProbed, this, &CameraWorker::onVideoFrameProbed);
    delete _videoProbe;
    _videoProbe = new QVideoProbe;
    _videoProbe->setSource(_camera);
    connect(_videoProbe, &QVideoProbe::videoFrameProbed, this, &CameraWorker::onVideoFrameProbed);

    QVideoEncoderSettings videoSettings;
    videoSettings.setCodec("h264");
    videoSettings.setResolution(352, 288);
    videoSettings.setQuality(QMultimedia::NormalQuality);
    videoSettings.setFrameRate(30);
    videoSettings.setBitRate(10);


    disconnect(_mediaRecorder, &QMediaRecorder::durationChanged, this, &CameraWorker::doVideoDurationChanged);
    disconnect(_mediaRecorder, QOverload<QMediaRecorder::Error>::of(&QMediaRecorder::error), this, &CameraWorker::onMediaRecorderError);
    disconnect(_mediaRecorder, &QMediaRecorder::stateChanged, this, &CameraWorker::doMediaRecorderStateChanged);
    delete _mediaRecorder;
    _mediaRecorder = new QMediaRecorder(_camera);
    _mediaRecorder->setVideoSettings(videoSettings);
    _mediaRecorder->setContainerFormat("mp4");
    connect(_mediaRecorder, &QMediaRecorder::durationChanged, this, &CameraWorker::doVideoDurationChanged);
    connect(_mediaRecorder, QOverload<QMediaRecorder::Error>::of(&QMediaRecorder::error), this, &CameraWorker::onMediaRecorderError);
    connect(_mediaRecorder, &QMediaRecorder::stateChanged, this, &CameraWorker::doMediaRecorderStateChanged);
    _camera->start();
    qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!CAMERA RESTART!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
}

void CameraWorker::capture()
{
    //    disconnect(_videoProbe, &QVideoProbe::videoFrameProbed, this, &CameraWorker::onVideoFrameProbed);
    QString photoFileName = "/sdcard/media/" + QDateTime::currentDateTime().toString("yyMMdd-hhmmss") + QString(".jpg");
    _cameraImageCapture->capture(photoFileName);
}

void CameraWorker::startCamera()
{
    _elapsedTimer.start();
    _camera->start();
    qDebug() << "CAMERA START";
}

void CameraWorker::stopCamera()
{
    _camera->stop();
    qDebug() << "CAMERA STOP";
}

void CameraWorker::startViewFinder()
{
    _isViewFinderEnabled.store(true);
}

void CameraWorker::stopViewFinder()
{
    _isViewFinderEnabled.store(false);
}

void CameraWorker::photoMode()
{
    if (_camera->captureMode() != QCamera::CaptureStillImage) {

        qDebug() << "=====================================PHOTO MODE===================================";
        _isChangingMode = true;
        _camera->stop();
        delay(1000);
        disconnect(_videoProbe, &QVideoProbe::videoFrameProbed, this, &CameraWorker::onVideoFrameProbed);
        _currentMode = QCamera::CaptureStillImage;
        _camera->setCaptureMode(QCamera::CaptureStillImage);
        connect(_videoProbe, &QVideoProbe::videoFrameProbed, this, &CameraWorker::onVideoFrameProbed);
        _camera->setViewfinder(_viewFinder);
        _camera->start();
    }
}

void CameraWorker::videoMode()
{
    if (_camera->captureMode() != QCamera::CaptureVideo) {
        qDebug() << "=====================================VIDEO MODE===================================";
        _isChangingMode = true;
        delay(1000);
        disconnect(_videoProbe, &QVideoProbe::videoFrameProbed, this, &CameraWorker::onVideoFrameProbed);
        _camera->stop();
        _currentMode = QCamera::CaptureVideo;
        _camera->setCaptureMode(QCamera::CaptureVideo);
        connect(_videoProbe, &QVideoProbe::videoFrameProbed, this, &CameraWorker::onVideoFrameProbed);
        _camera->setViewfinder(_viewFinder);
        _camera->start();
    }
    emit doCameraModeChanged();
}

void CameraWorker::onReadyForCaptureChanged(bool ready)
{
    emit doReadyForCaptureChanged(ready);
}

void CameraWorker::onImageCaptured(int id, const QImage& preview)
{
    QImage image(preview);
    emit doImageCaptured(id, image);
    //    connect(_videoProbe, &QVideoProbe::videoFrameProbed, this, &CameraWorker::onVideoFrameProbed);
}

void CameraWorker::onVideoFrameProbed(const QVideoFrame& frame)
{
    qDebug() << frame.size() << _currentMode;
    if (_isChangingMode) {
        _isChangingMode = false;
        emit doCameraModeChanged();
    }
    if (_elapsedTimer.elapsed() <= 40) {
        return;
    }
    if (frame.isValid() && _isViewFinderEnabled) {
        QImage image = qt_imageFromVideoFrame(frame);
        emit doImageViewfinder(image);
        _elapsedTimer.restart();
    }
}

void CameraWorker::onImageSaved(int id, const QString& fileName)
{
    qDebug() << "IMAGE SAVED TO: " << fileName;
    QString imageFileName = fileName;
    emit doImageSaved(imageFileName);
}

void CameraWorker::onMediaRecorderError(QMediaRecorder::Error error)
{
    qDebug() << _mediaRecorder->errorString();
}

void CameraWorker::onCameraStatusChanged(QCamera::Status status)
{
    //    if (_camera->status() == )
    qDebug() << "========STATUS: " << status;
}

void CameraWorker::onCameraStateChanged(QCamera::State state)
{
    if (state == QCamera::LoadedState) {
//        checkSupportedViewfinderResolutions();
    }
    qDebug() << "========STATE: " << state;
}

void CameraWorker::onCameraLockStatusChanged(QCamera::LockType lock, QCamera::LockStatus status, QCamera::LockChangeReason reason)
{
    qDebug() << "===== LOCK STATUS" << lock << status << reason;
}

void CameraWorker::onCameraError(QCamera::Error)
{
    qDebug() << "!!!!!!!!!!!!!!CAMERA ERROR!!!!!!!!!!!!" << _camera->errorString();
}

void CameraWorker::onCaptureModeChanged(QCamera::CaptureModes mode)
{
    qDebug() << "====================================CAPTURE MODE: " << mode;
}

void CameraWorker::onZoomIn()
{
    if (_cameraFocus->digitalZoom() != _cameraFocus->maximumDigitalZoom()) {
        QThread::msleep(100);
        _cameraFocus->zoomTo(1.0, _cameraFocus->digitalZoom() + 1.0);
        QThread::msleep(100);
    }
}

void CameraWorker::onZoomOut()
{
    if (_cameraFocus->digitalZoom() != 1.0) {
        QThread::msleep(100);
        _cameraFocus->zoomTo(1.0, _cameraFocus->digitalZoom() - 1.0);
        QThread::msleep(100);
    }
}

void CameraWorker::onRecord()
{
    if (_mediaRecorder->state() == QMediaRecorder::StoppedState || _mediaRecorder->state() == QMediaRecorder::PausedState) {
        _videoFileName = "/sdcard/media/" + QDateTime::currentDateTime().toString("yyMMdd-hhmmss") + QString(".mp4");
        _mediaRecorder->setOutputLocation(QUrl::fromLocalFile(_videoFileName));
        _mediaRecorder->record();
    }
    else {
        _mediaRecorder->stop();
    }
}

void CameraWorker::onStop()
{
    if (_mediaRecorder->state() == QMediaRecorder::RecordingState) {
        _mediaRecorder->stop();
        emit doVideoSaved(_videoFileName);
    }
}

void CameraWorker::onPhotoMode()
{
    if (_camera->status() == QCamera::ActiveStatus && _camera->state() == QCamera::ActiveState) {
        _camera->unlock();
        _camera->setCaptureMode(QCamera::CaptureStillImage);        
    }
}

void CameraWorker::onVideoMode()
{
    if (_camera->status() == QCamera::ActiveStatus && _camera->state() == QCamera::ActiveState) {        
        _camera->unlock();
        _camera->setCaptureMode(QCamera::CaptureVideo);        
    }
}

void CameraWorker::onReload()
{
    _camera->unlock();
    _camera->stop();
    _camera->unload();
    _camera->load();
    _camera->start();
    _camera->unlock();
}

void CameraWorker::checkSupportedImageSettings()
{
    // image codecs
    qDebug() << "IMAGE SETTINGS:";
    const QStringList supportedImageCodecs = _cameraImageCapture->supportedImageCodecs();
    for (const QString& codecName : supportedImageCodecs) {
        QString description = _cameraImageCapture->imageCodecDescription(codecName);
        qDebug() << "CODEC: " << description;
    }

    const QList<QSize> supportedResolutions = _cameraImageCapture->supportedResolutions();
    for (const QSize& resolution : supportedResolutions) {
        qDebug() << "RESOLUTION: " << resolution;
    }
}

void CameraWorker::checkSupportedVideoSettings()
{
    // audio codecs
    qDebug() << "VIDEO SETTINGS:";
    const QStringList supportedAudioCodecs = _mediaRecorder->supportedAudioCodecs();
    for (const QString& codecName : supportedAudioCodecs) {
        QString description = _mediaRecorder->audioCodecDescription(codecName);
        qDebug() << "AUDIO CODEC: " << description << codecName;
    }

    // sample rate:
    const QList<int> supportedAudioSampleRates = _mediaRecorder->supportedAudioSampleRates();
    for (int sampleRate : supportedAudioSampleRates) {
        qDebug() << "RATE: " << sampleRate;
    }

    // video codecs
    const QStringList supportedVideoCodecs = _mediaRecorder->supportedVideoCodecs();
    for (const QString& codecName : supportedVideoCodecs) {
        QString description = _mediaRecorder->videoCodecDescription(codecName);
        qDebug() << "VIDEO CODEC: " << description << codecName;
    }

    const QList<QSize> supportedResolutions = _mediaRecorder->supportedResolutions();
    for (const QSize& resolution : supportedResolutions) {
        qDebug() << "RESOLUTION: " << resolution;
    }

    const QList<qreal> supportedFrameRates = _mediaRecorder->supportedFrameRates();
    for (qreal rate : supportedFrameRates) {
        qDebug() << "RATE: " << rate;
    }

    // containers
    const QStringList formats = _mediaRecorder->supportedContainers();
    for (const QString& format : formats) {
        qDebug() << "FORMAT: " << format << _mediaRecorder->containerDescription(format);
    }
}

void CameraWorker::checkSupportedViewfinderResolutions()
{
    const QList<QSize> supportedResolutions = _camera->supportedViewfinderResolutions();
    for (const QSize& resolution : supportedResolutions) {
        qDebug() << "VIEWFINDER RESOLUTION: " << resolution;
    }
}
