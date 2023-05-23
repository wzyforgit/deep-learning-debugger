// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "alg/detect/yolov5s.h"
#include "alg/detect/yolact.h"

#include <QWidget>

class ImageView;
class QTextEdit;
class CameraControl;
class QTimer;
class QLabel;

class DetectPage : public QWidget
{
    Q_OBJECT

public:
    DetectPage(QWidget *parent = nullptr);
    ~DetectPage() = default;

    QString pageName() const
    {
        return tr("目标检测");
    }

private:
    void initUI();
    void openImage(const QImage &image, bool useFps);
    void openImage(const QString &path);
    void addMessage(const QString &msg);

    QImage drawOnYolov5s(const QImage &srcImage, const QList<Yolov5s::DetectResult> &result);
    QImage drawOnYolact(const QImage &srcImage, const QList<Yolact::DetectResult> &result);

    ImageView *srcView;
    ImageView *dstView;
    QTextEdit *msgBox;
    Yolov5s *yolov5s;
    Yolact *yolact;
    CameraControl *camera;
    QTimer *cameraFlushTimer;
    QLabel *fpsLabel;
};
