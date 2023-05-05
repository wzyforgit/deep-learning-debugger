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

    //设置分辨率
    void setResolution(int width, int height);

    //设置捕获
    void setCaptureInterval(int ms);

    //打开摄像机
    bool openCamera(int id);

    //关闭摄像机
    bool closeCamera();

    //摄像机是否在工作中
    bool isWorking();

    //摄像机采集到的最后一帧画面
    QImage lastImage() const;

private:
    CameraControlThread *capWorker;
};
