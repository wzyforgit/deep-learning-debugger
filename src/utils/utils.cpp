// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <sys/time.h>
#include <QFile>
#include <QtDebug>

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

bool createPaddleSpeechWaveFile(const QString &fileName, const QAudioFormat &format)
{
    //PaddleSpeech要求数据的采样率必须是16k，wav文件要求数据必须是小端
    //此处图省事，直接转到float，再一次转到16k的采样率

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "file open failed";
        return false;
    }
    auto data = file.readAll();
    file.close();

    //1.准备数据
    auto fusedData = sampleDataFusion(data, format);
    auto stdSampleRateData = changeSampleRate(fusedData, format.sampleRate(), 16000);

    QVector<qint16> stdSaveData;
    std::transform(stdSampleRateData.begin(), stdSampleRateData.end(), std::back_inserter(stdSaveData), [](qreal data){
        return static_cast<qint16>(data * 32768);
    });

    QByteArray dstData(stdSaveData.size() * sizeof(qint16), '\0');
    memcpy(dstData.data(), stdSaveData.data(), dstData.size());

    //2.写入文件头
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "file open failed";
        return false;
    }

    unsigned int intBuffer;

    //2.1 写入RIFF
    file.write("\x52\x49\x46\x46", 4); //RIFF
    intBuffer = 36 + dstData.size();
    file.write(reinterpret_cast<char*>(&intBuffer), 4); //file size - 8
    file.write("\x57\x41\x56\x45", 4); //WAVE

    //2.2 写入format
    file.write("\x66\x6D\x74\x20", 4); //'fmt '
    file.write("\x10\x00\x00\x00", 4); //16
    file.write("\x01\x00", 2); //Audio Format 01:pcm 03:ieee float
    file.write("\x01\x00", 2); //Num Channels
    file.write("\x80\x3E\x00\x00", 4); //Sample rate
    file.write("\x00\x7D\x00\x00", 4); //SampleRate * NumChannels * BitsPerSample / 8
    file.write("\x02\x00", 2); //NumChannels * BitsPerSample / 8
    file.write("\x10\x00", 2); //8:8bit, 16:16bit, 32:32bit

    //2.3 写入data
    file.write("\x64\x61\x74\x61", 4); //data
    intBuffer = dstData.size();
    file.write(reinterpret_cast<char*>(&intBuffer), 4); //dstData.size()
    file.write(dstData);

    return file.waitForBytesWritten(-1);
}

const QList<QColor>& drawPalette()
{
    static const QList<QColor> colors = {
        {56, 0, 255}, {226, 255, 0}, {0, 94, 255}, {0, 37, 255}, {0, 255, 94},
        {255, 226, 0}, {0, 18, 255}, {255, 151, 0}, {170, 0, 255}, {0, 255, 56},
        {255, 0, 75}, {0, 75, 255}, {0, 255, 169}, {255, 0, 207}, {75, 255, 0},
        {207, 0, 255}, {37, 0, 255}, {0, 207, 255}, {94, 0, 255}, {0, 255, 113},
        {255, 18, 0}, {255, 0, 56}, {18, 0, 255}, {0, 255, 226}, {170, 255, 0},
        {255, 0, 245}, {151, 255, 0}, {132, 255, 0}, {75, 0, 255}, {151, 0, 255},
        {0, 151, 255}, {132, 0, 255}, {0, 255, 245}, {255, 132, 0}, {226, 0, 255},
        {255, 37, 0}, {207, 255, 0}, {0, 255, 207}, {94, 255, 0}, {0, 226, 255},
        {56, 255, 0}, {255, 94, 0}, {255, 113, 0}, {0, 132, 255}, {255, 0, 132},
        {255, 170, 0}, {255, 0, 188}, {113, 255, 0}, {245, 0, 255}, {113, 0, 255},
        {255, 188, 0}, {0, 113, 255}, {255, 0, 0}, {0, 56, 255}, {255, 0, 113},
        {0, 255, 188}, {255, 0, 94}, {255, 0, 18}, {18, 255, 0}, {0, 255, 132},
        {0, 188, 255}, {0, 245, 255}, {0, 169, 255}, {37, 255, 0}, {255, 0, 151},
        {188, 0, 255}, {0, 255, 37}, {0, 255, 0}, {255, 0, 170}, {255, 0, 37},
        {255, 75, 0}, {0, 0, 255}, {255, 207, 0}, {255, 0, 226}, {255, 245, 0},
        {188, 255, 0}, {0, 255, 18}, {0, 255, 75}, {0, 255, 151}, {255, 56, 0},
        {245, 255, 0}
    };
    return colors;
}
