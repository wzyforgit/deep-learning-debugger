// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <ncnn/mat.h>

namespace ncnn {
    class Net;
}

class Yolov5s
{
public:
    struct DetectResult
    {
        QRectF bbox; //矩形框
        float prob; //矩形框置信度
        int cls; //分类结果标签
        float clsProb; //分类结果置信度
        QString clsStr; //分类结果对应的文本
    };

    explicit Yolov5s();
    ~Yolov5s();

    void init();
    void setImage(const QImage &image);
    void analyze();
    QList<DetectResult> result() const;

private:
    QList<DetectResult> generateDetectResult(const ncnn::Mat &out, float bboxThreshold, float clsThreshold);
    float iou(const QRectF &lhs, const QRectF &rhs);
    QList<DetectResult> nms(const QList<DetectResult> &originResults, int clsCount, float iouThreshold);

    QImage imageCache;
    QList<DetectResult> lastResult;
    ncnn::Net *net = nullptr;
    QStringList dict;
};
