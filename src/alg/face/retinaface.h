// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <ncnn/mat.h>

namespace ncnn {
    class Net;
}

class RetinaFace
{
public:
    struct DetectResult
    {
        QRectF bbox; //矩形框
        float prob; //矩形框置信度
        float landmark[10]; //人脸关键点
    };

    RetinaFace();
    ~RetinaFace();

    void init();
    void setImage(const QImage &image);
    void analyze();
    QVector<DetectResult> result() const;

private:
    QImage imageCache;
    QVector<DetectResult> lastResult;
    ncnn::Net *net = nullptr;
};
