// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <opencv2/core/core.hpp>

namespace ncnn {
    class Net;
}

class Yolact
{
public:
    struct DetectResult
    {
        cv::Rect_<float> rect;
        QString labelStr;
        int label;
        float prob;
        std::vector<float> maskdata;
        cv::Mat mask;
    };

    Yolact() = default;
    ~Yolact();

    void init();
    void setImage(const QImage &image);
    void analyze();
    QList<DetectResult> result() const;

private:
    QImage imageCache;
    QList<DetectResult> lastResult;
    ncnn::Net *net = nullptr;
    QStringList dict;
};
