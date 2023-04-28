// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cameracontrol.h"
#include "utils.h"

#include <QThread>
#include <QMutex>
#include <QImage>

#include <atomic>

#include <opencv2/videoio.hpp>

static QMutex mutex;
static QImage imageCache;

class CameraControlThread : public QThread
{
    Q_OBJECT

public:
    CameraControlThread(QObject *parent = nullptr);
    void setCaptureInterval(int ms);
    bool openCamera(int id);
    void setResolution(int width, int height);

protected:
    void run() override;

private:
    std::atomic_int captureInterval = 50;
    cv::VideoCapture cap;
};

CameraControlThread::CameraControlThread(QObject *parent)
    : QThread(parent)
{
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
}

void CameraControlThread::setCaptureInterval(int ms)
{
    captureInterval = ms;
}

bool CameraControlThread::openCamera(int id)
{
    return cap.open(id);
}

void CameraControlThread::setResolution(int width, int height)
{
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
}

void CameraControlThread::run()
{
    cv::Mat mat;
    while(1)
    {
        cap >> mat;
        auto image = cvMat2QImage(mat);

        mutex.lock();
        imageCache = image;
        mutex.unlock();

        if(isInterruptionRequested())
        {
            break;
        }
        QThread::msleep(captureInterval);
    }
    cap.release();
}

CameraControl::CameraControl(QObject *parent)
    : QObject(parent)
    , capWorker(new CameraControlThread(this))
{
}

CameraControl::~CameraControl()
{
    closeCamera();
}

void CameraControl::setResolution(int width, int height)
{
    capWorker->setResolution(width, height);
}

void CameraControl::setCaptureInterval(int ms)
{
    capWorker->setCaptureInterval(ms);
}

bool CameraControl::openCamera(int id)
{
    if(capWorker->isRunning() || !capWorker->openCamera(id))
    {
        return false;
    }

    capWorker->start();
    return true;
}

bool CameraControl::closeCamera()
{
    if(!capWorker->isRunning())
    {
        return false;
    }
    capWorker->requestInterruption();
    return true;
}

bool CameraControl::isWorking()
{
    return capWorker->isRunning();
}

QImage CameraControl::lastImage() const
{
    mutex.lock();
    auto image = imageCache;
    mutex.unlock();

    return image;
}

#include "cameracontrol.moc"
