// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

//本页的模型采用的是insight face项目下的arcface r100预训练模型，使用mxnet2ncnn和ncnnoptimize进行转换和优化

#include "arcface.h"

#include <ncnn/net.h>
#include <cmath>

ArcFace::~ArcFace()
{
    delete net;
}

void ArcFace::init()
{
    net = new ncnn::Net;
    net->opt.use_vulkan_compute = true;
    net->load_param("/home/fuko/桌面/AI/face/arc-opt.param");
    net->load_model("/home/fuko/桌面/AI/face/arc-opt.bin");
}

void ArcFace::setImage(const QImage &image)
{
    imageCache = image.convertToFormat(QImage::Format_RGB888);
}

void ArcFace::analyze()
{
    if(net == nullptr)
    {
        init();
    }

    //归一化已经集成在模型中，此处只需要传入图片即可
    ncnn::Mat input = ncnn::Mat::from_pixels(imageCache.bits(), ncnn::Mat::PIXEL_RGB, imageCache.width(), imageCache.height(), imageCache.bytesPerLine());
    auto extractor = net->create_extractor();
    extractor.input("id", input);
    ncnn::Mat output;
    extractor.extract("fc1", output);

    QVector<float> result;
    auto ptr = output.channel(0);
    for(int i = 0;i != output.w;++i)
    {
        result.push_back(ptr[i]);
    }

    lastResult = result;
}

QVector<float> ArcFace::result() const
{
    return lastResult;
}

float ArcFace::compare(const QVector<float> &lhs, const QVector<float> &rhs)
{
    if(lhs.size() != rhs.size())
    {
        return 0;
    }

    int featLen = lhs.size();
    float sumAB = 0;
    float modA = 0;
    float modB = 0;
    for(int i = 0;i != featLen;++i)
    {
        sumAB += lhs[i] * rhs[i];
        modA += lhs[i] * lhs[i];
        modB += rhs[i] * rhs[i];
    }
    modA = std::sqrt(modA);
    modB = std::sqrt(modB);

    return (sumAB / (modA * modB)) * 0.5 + 0.5;
}
