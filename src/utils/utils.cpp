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

QVector<qreal> sampleDataFusion_8bit(const QByteArray &data, const QAudioFormat &format)
{
    QVector<qreal> result;

    if(format.sampleType() == QAudioFormat::SignedInt)
    {
        std::transform(data.begin(), data.end(), std::back_inserter(result), [](int8_t currentData){
            return currentData / 128.0;
        });
    }
    else
    {
        std::transform(data.begin(), data.end(), std::back_inserter(result), [](uint8_t currentData){
            return (currentData - 127.5) / 127.5;
        });
    }

    return result;
}

QVector<qreal> sampleDataFusion_16bit(const QByteArray &data, const QAudioFormat &format)
{
    QVector<qreal> result;

    char dataBuffer[2];
    for(int i = 0;i != data.size();i += 2)
    {
        if(format.byteOrder() == QAudioFormat::BigEndian)
        {
            dataBuffer[0] = data[i + 1];
            dataBuffer[1] = data[i];
        }
        else
        {
            dataBuffer[0] = data[i];
            dataBuffer[1] = data[i + 1];
        }

        if(format.sampleType() == QAudioFormat::SignedInt)
        {
            int16_t *realDataPtr = reinterpret_cast<int16_t*>(dataBuffer);
            int16_t realData = *realDataPtr;
            result.push_back(realData / 32768.0);
        }
        else
        {
            uint16_t *realDataPtr = reinterpret_cast<uint16_t*>(dataBuffer);
            uint16_t realData = *realDataPtr;
            result.push_back((realData - 32767.5) / 32767.5);
        }
    }

    return result;
}

QVector<qreal> sampleDataFusion_24bit(const QByteArray &data, const QAudioFormat &format)
{
    QVector<qreal> result;

    char dataBuffer[4];
    for(int i = 0;i != data.size();i += 3)
    {
        if(format.byteOrder() == QAudioFormat::BigEndian)
        {
            dataBuffer[0] = data[i + 2];
            dataBuffer[1] = data[i + 1];
            dataBuffer[2] = data[i];
        }
        else
        {
            dataBuffer[0] = data[i];
            dataBuffer[1] = data[i + 1];
            dataBuffer[2] = data[i + 2];
        }

        dataBuffer[3] = dataBuffer[2] & 0x80 ? 0xff : 0x00; //额外处理负数情况

        if(format.sampleType() == QAudioFormat::SignedInt)
        {
            int32_t *realDataPtr = reinterpret_cast<int32_t*>(dataBuffer);
            int32_t realData = *realDataPtr;
            result.push_back(realData / 8388608.0);
        }
        else
        {
            uint32_t *realDataPtr = reinterpret_cast<uint32_t*>(dataBuffer);
            uint32_t realData = *realDataPtr;
            result.push_back((realData - 8388607.5) / 8388607.5);
        }
    }

    return result;
}

QVector<qreal> sampleDataFusion_32bit(const QByteArray &data, const QAudioFormat &format)
{
    QVector<qreal> result;

    char dataBuffer[4];
    for(int i = 0;i != data.size();i += 4)
    {
        if(format.byteOrder() == QAudioFormat::BigEndian)
        {
            dataBuffer[0] = data[i + 3];
            dataBuffer[1] = data[i + 2];
            dataBuffer[2] = data[i + 1];
            dataBuffer[3] = data[i];
        }
        else
        {
            dataBuffer[0] = data[i];
            dataBuffer[1] = data[i + 1];
            dataBuffer[2] = data[i + 2];
            dataBuffer[3] = data[i + 3];
        }

        if(format.sampleType() == QAudioFormat::SignedInt)
        {
            int32_t *realDataPtr = reinterpret_cast<int32_t*>(dataBuffer);
            int32_t realData = *realDataPtr;
            result.push_back(realData / 2147483648.0);
        }
        else if(format.sampleType() == QAudioFormat::UnSignedInt)
        {
            uint32_t *realDataPtr = reinterpret_cast<uint32_t*>(dataBuffer);
            uint32_t realData = *realDataPtr;
            result.push_back((realData - 2147483647.5) / 2147483647.5);
        }
        else
        {
            float *realDataPtr = reinterpret_cast<float*>(dataBuffer);
            float realData = *realDataPtr;
            result.push_back(realData);
        }
    }

    return result;
}

//目前只考虑本机是小端机的情况
QVector<qreal> sampleDataFusion(const QByteArray &data, const QAudioFormat &format)
{
    if(format.sampleType() == QAudioFormat::Unknown || format.sampleSize() == 64)
    {
        return QVector<qreal>();
    }

    if(format.sampleSize() == 8)
    {
        return sampleDataFusion_8bit(data, format);
    }

    switch(format.sampleSize())
    {
    default:
        return QVector<qreal>();
    case 8:
        return sampleDataFusion_8bit(data, format);
    case 16:
        return sampleDataFusion_16bit(data, format);
    case 24:
        return sampleDataFusion_24bit(data, format);
    case 32:
        return sampleDataFusion_32bit(data, format);
    }
}
