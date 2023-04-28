// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <sys/time.h>

//摘抄自：https://github.com/Tencent/ncnn/blob/master/src/benchmark.cpp 下的 double get_current_time() 函数
double getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

QImage cvMat2QImage(const cv::Mat &mat)
{
    //只处理这两种情况
    QImage::Format imageFormat = mat.type() == CV_8UC4 ? QImage::Format_RGB32 : QImage::Format_RGB888;
    return QImage(mat.data, mat.cols, mat.rows, mat.step, imageFormat).rgbSwapped();
}
