#include "cameraworker.h"
#include <private/qvideoframe_p.h>


CameraWorker::CameraWorker(QObject* parent)
    : QObject(parent)
{
}

void CameraWorker::init()
{
    _camera = new QCamera(QCamera::BackFace);
    _camera->setCaptureMode(QCamera::CaptureStillImage);

    _cameraFocus = _camera->focus();
    _cameraFocus->setFocusMode(QCameraFocus::AutoFocus);

    QCameraViewfinder* viewFinder = new QCameraViewfinder;
    _camera->setViewfinder(viewFinder);

    _cameraImageCapture = new QCameraImageCapture(_camera);
    connect(_cameraImageCapture, &QCameraImageCapture::readyForCaptureChanged, this, &CameraWorker::onReadyForCaptureChanged);
    connect(_cameraImageCapture, &QCameraImageCapture::imageCaptured, this, &CameraWorker::onImageCaptured);

    _videoProbe = new QVideoProbe;
    connect(_videoProbe, &QVideoProbe::videoFrameProbed, this, &CameraWorker::onVideoFrameProbed);
    _videoProbe->setSource(_camera);

    _camera->start();
}

void CameraWorker::capture()
{
    _cameraImageCapture->capture();
}

void CameraWorker::onReadyForCaptureChanged(bool ready)
{
    emit doReadyForCaptureChanged(ready);
}

void CameraWorker::onImageCaptured(int id, const QImage& preview)
{
    QImage image(preview);
    emit doImageCaptured(id, image);
}

void CameraWorker::onVideoFrameProbed(const QVideoFrame& frame)
{
    if (frame.isValid()) {
        QVideoFrame cloneFrame(frame);
        qDebug() << "Mapped: " << cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        QByteArray byteArray((char*) cloneFrame.bits());
        if (byteArray.size() != cloneFrame.mappedBytes()) {
            return;
        }
        qDebug() << "ByteArray size: " << byteArray.size();
        qDebug() << "Mapped Bytes: " << cloneFrame.mappedBytes();
        //        QImage image((uchar*) byteArray.data(), cloneFrame.width(), cloneFrame.height(), cloneFrame.bytesPerLine(), QImage::Format_ARGB32);

        qDebug() << "IMAGE SIZE: " << image.size();
        //                emit doImageViewfinder(image);
        cloneFrame.unmap();
        //        delete[] data;
        qDebug() << "UnMapped!";
    }
}
