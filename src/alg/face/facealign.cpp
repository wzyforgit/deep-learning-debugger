// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "facealign.h"

#include <ncnn/mat.h>

void FaceAlign::setImage(const QImage &image)
{
    imageCache = image.convertToFormat(QImage::Format_RGB888);
}

//改编自ncnn wiki
void FaceAlign::analyze(float *currentLandmark)
{
    static float pointDst[10] = { // +8 是因为我们处理112*112的图，不加则是 112*96
        30.2946f + 8.0f, 51.6963f,
        65.5318f + 8.0f, 51.5014f,
        48.0252f + 8.0f, 71.7366f,
        33.5493f + 8.0f, 92.3655f,
        62.7299f + 8.0f, 92.2041f,
    };

    //计算变换矩阵
    float tm_inv[6];
    ncnn::get_affine_transform(pointDst, currentLandmark, 5, tm_inv);

    //计算结果存放位置
    QImage resultImg(112, 112, QImage::Format_RGB888);

    //执行变换，结果会直接刷入resultImg
    ncnn::warpaffine_bilinear_c3(imageCache.bits(), imageCache.width(), imageCache.height(),
                                 resultImg.bits(), resultImg.width(), resultImg.height(),
                                 tm_inv);

    //获取变换结果
    lastResult = resultImg;
}

QImage FaceAlign::result() const
{
    return lastResult;
}
