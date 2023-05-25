// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>

namespace ncnn {
    class Net;
}

class Yolov8nPose
{
public:
    struct KpsData
    {
        float score;
        QPointF point;
    };

    struct DetectResult
    {
        float score;
        QRectF bbox;
        QList<KpsData> kps;
    };

    Yolov8nPose() = default;
    ~Yolov8nPose();

    void init();
    void setImage(const QImage &image);
    void analyze();
    QList<DetectResult> result() const;
    static const QList<QPair<int, int>> &skeleton();
    static const QList<QColor> &kpsColors();
    static const QList<QColor> &limbColors();

private:
    ncnn::Net *net = nullptr;
    QImage imageCache;
    QList<DetectResult> lastResult;
};
