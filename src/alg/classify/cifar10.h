// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <QMap>

namespace ncnn {
   class Net;
}

class Cifar10
{
public:
    struct ClassifyResult
    {
        QList<float> probs; //所有标签下的置信度
        QList<QPair<QString, float>> clsPair; //按从高到底排序的标签+置信度
    };

    explicit Cifar10();
    ~Cifar10();

    void init();
    void setImage(const QImage &image);
    void analyze();
    ClassifyResult result() const;

    QStringList clsStrs() const; //分类标签合集

private:
    ncnn::Net *net = nullptr;
    ClassifyResult lastResult;
    QStringList clsLabel;
    QImage imageCache;
};
