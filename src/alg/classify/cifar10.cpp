// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cifar10.h"

#include <ncnn/net.h>

Cifar10::Cifar10()
{
    clsLabel = QStringList{"plane", "car", "bird", "cat", "deer",
                           "dog", "frog", "horse", "ship", "truck"};
}

Cifar10::~Cifar10()
{
    delete net;
}

void Cifar10::init()
{
    //加载模型
    net = new ncnn::Net;
    net->opt.use_vulkan_compute = true;
    net->load_param("/home/fuko/桌面/AI/Classify/cifar10.param");
    net->load_model("/home/fuko/桌面/AI/Classify/cifar10.bin");
}

void Cifar10::setImage(const QImage &image)
{
    imageCache = image.convertToFormat(QImage::Format_RGB888);
}

void Cifar10::analyze()
{
    if(net == nullptr)
    {
        init();
    }

    ncnn::Mat input = ncnn::Mat::from_pixels_resize(imageCache.bits(), ncnn::Mat::PIXEL_RGB, imageCache.width(), imageCache.height(), imageCache.bytesPerLine(), 32, 32);
    float mean[] = { 128.f, 128.f, 128.f };
    float norm[] = { 1/128.f, 1/128.f, 1/128.f };
    input.substract_mean_normalize(mean, norm);

    auto ex = net->create_extractor();
    ex.input("in0", input);

    ncnn::Mat out;
    ex.extract("out0", out);

    QList<float> clsProb;
    QList<QPair<QString, float>> clsPair;
    auto ptr = out.channel(0);
    for(int i = 0;i != out.w;i++)
    {
        clsProb.push_back(ptr[i]);
        clsPair.push_back(QPair(clsLabel[i], ptr[i]));
    }
    std::sort(clsPair.begin(), clsPair.end(), [](const QPair<QString, float> &lhs, const QPair<QString, float> &rhs){
        return lhs.second > rhs.second;
    });

    lastResult.probs = clsProb;
    lastResult.clsPair = clsPair;
}

Cifar10::ClassifyResult Cifar10::result() const
{
    return lastResult;
}

QStringList Cifar10::clsStrs() const
{
    return clsLabel;
}
