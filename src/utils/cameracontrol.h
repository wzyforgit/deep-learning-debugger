// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class CameraControlThread;

class CameraControl : public QObject
{
    Q_OBJECT
public:
    explicit CameraControl(QObject *parent = nullptr);
    ~CameraControl() override;

    void setResolution(int width, int height);
    void setCaptureInterval(int ms);
    bool openCamera(int id);
    bool closeCamera();
    bool isWorking();
    QImage lastImage() const;

private:
    CameraControlThread *capWorker;
};
