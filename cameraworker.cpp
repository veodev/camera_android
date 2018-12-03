#include "cameraworker.h"
#include <private/qvideoframe_p.h>
#include <QKeyEvent>

CameraWorker::CameraWorker(QObject* parent)
    : QObject(parent)    
{
}

void CameraWorker::init()
{    
    _isViewFinderEnabled = true;
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

    _elapsedTimer = new QElapsedTimer;
    _elapsedTimer->start();

    _camera->start();
}

void CameraWorker::capture()
{    
    _cameraImageCapture->capture();
}

void CameraWorker::startCamera()
{
    _camera->start();
}

void CameraWorker::stopCamera()
{
    _camera->stop();
}

void CameraWorker::startViewFinder()
{
    _isViewFinderEnabled.store(true);
}

void CameraWorker::stopViewFinder()
{
    _isViewFinderEnabled.store(false);
}

void CameraWorker::imageCleanupHandler(void *info)
{
    auto* ptr = reinterpret_cast<uchar*>(info);
    delete ptr;
}

int *CameraWorker::convertYUV420_NV21toRGB8888(unsigned char data[], int width, int height)
{
    int size = width*height;
        int offset = size;
        int* pixels = new int[size];
        int u, v, y1, y2, y3, y4;

        // i percorre os Y and the final pixels
        // k percorre os pixles U e V
        for(int i=0, k=0; i < size; i+=2, k+=2) {
            y1 = data[i  ]&0xff;
            y2 = data[i+1]&0xff;
            y3 = data[width+i  ]&0xff;
            y4 = data[width+i+1]&0xff;

            u = data[offset+k  ]&0xff;
            v = data[offset+k+1]&0xff;
            u = u-128;
            v = v-128;

            pixels[i  ] = convertYUVtoRGB(y1, u, v);
            pixels[i+1] = convertYUVtoRGB(y2, u, v);
            pixels[width+i  ] = convertYUVtoRGB(y3, u, v);
            pixels[width+i+1] = convertYUVtoRGB(y4, u, v);

            if (i!=0 && (i+2)%width==0)
                i+=width;
        }

        return pixels;
}

int CameraWorker::convertYUVtoRGB(int y, int u, int v)
{
    int r,g,b;
    r = y + (int)(1.402f*v);
    g = y - (int)(0.344f*u +0.714f*v);
    b = y + (int)(1.772f*u);

    r = r>255? 255 : r<0 ? 0 : r;
    g = g>255? 255 : g<0 ? 0 : g;
    b = b>255? 255 : b<0 ? 0 : b;
    return 0xff000000 | (b<<16) | (g<<8) | r;
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
    if(_elapsedTimer->elapsed() <= 40 || _isViewFinderEnabled == false) {
        return;
    }

    if (frame.isValid()) {        
        QImage image = qt_imageFromVideoFrame(frame);
        emit doImageViewfinder(image);
//        QVideoFrame cloneFrame(frame);
//        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
//        int* data = convertYUV420_NV21toRGB8888(cloneFrame.bits(), cloneFrame.width(), cloneFrame.height());
//        emit doImageViewfinder(QImage(reinterpret_cast<uchar*>(data), cloneFrame.width(), cloneFrame.height(), QImage::Format_ARGB32, imageCleanupHandler, data));
//        cloneFrame.unmap();
        _elapsedTimer->restart();
    }
}


