// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class ImageView;
class QTextEdit;
class CameraControl;
class QTimer;
class QLabel;
class QListView;
class FaceView;
class RetinaFace;
class FaceAlign;
class ArcFace;

class FacePage : public QWidget
{
    Q_OBJECT

public:
    explicit FacePage(QWidget *parent = nullptr);

    QString pageName() const
    {
        return tr("人脸识别");
    }

private:
    void initUI();
    void openImage(const QImage &image, bool useFps);
    void openImage(const QString &path);
    void recFaces();
    void addMessage(const QString &msg);

    ImageView *srcView;
    ImageView *dstView;
    QTextEdit *msgBox;
    CameraControl *camera;
    QTimer *cameraFlushTimer;
    QLabel *fpsLabel;
    FaceView *faceChooseView;

    RetinaFace *retinaFace;
    FaceAlign  *faceAlign;
    ArcFace    *arcFace;

    QList<QImage> alignFaces;
};
