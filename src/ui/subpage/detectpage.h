// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class ImageView;
class QTextEdit;
class CameraControl;
class Yolov5s;
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

    ImageView *srcView;
    ImageView *dstView;
    QTextEdit *msgBox;
    Yolov5s *yolov5s;
    CameraControl *camera;
    QTimer *cameraFlushTimer;
    QLabel *fpsLabel;
};
