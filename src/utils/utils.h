// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <opencv2/opencv.hpp>

//提供毫秒级时间计算
double getCurrentTime();

//测算函数的执行时间
template <typename Func>
void runWithTime(Func &&f, double *timeUsed)
{
    double start = getCurrentTime();
    f();
    *timeUsed = getCurrentTime() - start;
}

//cv::Mat -> QImage
QImage cvMat2QImage(const cv::Mat &mat);
