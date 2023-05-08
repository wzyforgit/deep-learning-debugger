// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <QVector>
#include <opencv2/opencv.hpp>

//提供毫秒级时间计算
extern double getCurrentTime();

//测算函数的执行时间
template <typename Func>
void runWithTime(Func &&f, double *timeUsed)
{
    double start = getCurrentTime();
    f();
    *timeUsed = getCurrentTime() - start;
}

//cv::Mat -> QImage
extern QImage cvMat2QImage(const cv::Mat &mat);

//对一维数据进行升降采样
template <typename T>
QVector<T> changeSampleRate(const QVector<T> &src, int fromRate, int toRate)
{
    //1.计算最终数据的数量
    qreal ratio = toRate / static_cast<qreal>(fromRate);
    int targetSize = static_cast<int>(src.size() * ratio);

    //2.执行转换，采用线性缩放
    qreal calRatio = (toRate - 1) / static_cast<qreal>(fromRate - 1);
    QVector<T> dst(targetSize);
    dst[0] = src[0];
    dst[dst.size() - 1] = src[src.size() - 1];
    for(int i = 1;i != dst.size() - 1;++i)
    {
        //虚拟位置
        qreal srcPos = i / calRatio;
        //前向位置
        int preSrcPos = static_cast<int>(srcPos);
        //后向位置
        int afterSrcPos = preSrcPos + 1;
        //点位的比率
        qreal posRatio = srcPos - preSrcPos;
        //点位的值
        dst[i] = static_cast<T>(src[preSrcPos] * (1.0 - posRatio) + src[afterSrcPos] * (posRatio));
    }

    return dst;
}
