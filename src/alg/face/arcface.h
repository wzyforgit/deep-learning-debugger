// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>

namespace ncnn {
    class Net;
}

class ArcFace
{
public:
    ArcFace() = default;
    ~ArcFace();
    void init();
    void setImage(const QImage &image); //需要预先执行五点对齐
    void analyze();
    QVector<float> result() const;
    static float compare(const QVector<float> &lhs, const QVector<float> &rhs); //值大于等于0.7即可认为是同一个人，严格环境下可以取到大于等于0.8或更高

private:
    QImage imageCache;
    QVector<float> lastResult;
    ncnn::Net *net = nullptr;
};
