// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiowaveview.h"
#include "utils/utils.h"

#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QLabel>
#include <QVBoxLayout>
#include <QtDebug>
#include <cmath>

#include <opencv2/opencv.hpp>

AudioWaveView::AudioWaveView(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void AudioWaveView::initUI()
{
    //频谱图
    specLineSeries = new QtCharts::QLineSeries;
    specXAxis = new QtCharts::QValueAxis;
    specYAxis = new QtCharts::QValueAxis;
    specChart = new QtCharts::QChart;

    specXAxis->setTitleText(tr("频率（Hz）"));
    specXAxis->setRange(0, 1);
    specXAxis->setLabelFormat("%g");

    specYAxis->setTitleText(tr("振幅（dB）"));
    specYAxis->setRange(-100, 100);
    specYAxis->setLabelFormat("%g");

    specChart->addAxis(specXAxis, Qt::AlignBottom);
    specChart->addAxis(specYAxis, Qt::AlignLeft);
    specChart->addSeries(specLineSeries);
    specLineSeries->attachAxis(specXAxis);
    specLineSeries->attachAxis(specYAxis);
    specChart->legend()->hide();
    specChart->setTitle(tr("频谱图"));

    auto specChartView = new QtCharts::QChartView;
    specChartView->setChart(specChart);
    specChartView->setRenderHint(QPainter::Antialiasing);

    //波形图
    waveLineSeries = new QtCharts::QLineSeries;
    waveXAxis = new QtCharts::QValueAxis;
    waveYAxis = new QtCharts::QValueAxis;
    waveChart = new QtCharts::QChart;

    waveXAxis->setTitleText(tr("时刻"));
    waveXAxis->setRange(0, 2);
    waveXAxis->setLabelFormat("%g");

    waveYAxis->setTitleText(tr("振幅"));
    waveYAxis->setRange(-1, 1);
    waveYAxis->setLabelFormat("%g");

    waveChart->addAxis(waveXAxis, Qt::AlignBottom);
    waveChart->addAxis(waveYAxis, Qt::AlignLeft);
    waveChart->addSeries(waveLineSeries);
    waveLineSeries->attachAxis(waveXAxis);
    waveLineSeries->attachAxis(waveYAxis);
    waveChart->legend()->hide();
    waveChart->setTitle(tr("波形图"));

    auto waveChartView = new QtCharts::QChartView;
    waveChartView->setChart(waveChart);
    waveChartView->setRenderHint(QPainter::Antialiasing);

    auto allLayer = new QVBoxLayout;
    allLayer->addWidget(specChartView);
    allLayer->addWidget(waveChartView);

    setLayout(allLayer);
}

void AudioWaveView::setAudioParam(const QAudioFormat &format)
{
    currentFormat = format;

    specPointCount = format.sampleRate() / 2;
    wavePointCount = format.sampleRate() / 4;

    specXAxis->setRange(0, specPointCount);
    specLineSeries->clear();
    waveXAxis->setRange(0, wavePointCount);
    waveLineSeries->clear();
}

QVector<QPointF> AudioWaveView::generateWavePoints(int pointCount, const QVector<QPointF> &oldPoints, const QVector<qreal> &data)
{
    QVector<QPointF> points;

    if(data.size() <= pointCount)
    {
        if(oldPoints.size() < pointCount)
        {
            if(oldPoints.size() + data.size() > pointCount)
            {
                int startCopyPos = oldPoints.size() - (pointCount - data.size());
                for(int i = startCopyPos;i < oldPoints.size();++i)
                {
                    points.push_back(QPointF(i - startCopyPos, oldPoints.at(i).y()));
                }
            }
            else
            {
                points = oldPoints;
            }
        }
        else
        {
            for(int i = data.size();i < oldPoints.size();++i)
            {
                points.push_back(QPointF(i - data.size(), oldPoints.at(i).y()));
            }
        }

        int startSize = points.size();
        for(int i = 0;i != data.size();++i)
        {
            points.push_back(QPointF(i + startSize, data.at(i)));
        }
    }
    else
    {
        int copyStart = data.size() - pointCount;
        for(int i = 0;i != pointCount;++i)
        {
            points.push_back(QPointF(i, data.at(copyStart + i)));
        }
    }

    return points;
}

QVector<QPointF> AudioWaveView::generateSpecPoints(int pointCount, const QVector<qreal> &data)
{
    qreal res = static_cast<qreal>(pointCount) / data.size();

    QVector<QPointF> points;
    for(int i = 0;i != data.size();++i)
    {
        //感觉原始数据好看一些，需要再研究
        //points.push_back(QPointF(i * res, 20 * std::log10(std::fabs(data.at(i))))); //分贝
        points.push_back(QPointF(i * res, std::fabs(data.at(i)))); //原始
    }

    return points;
}

void AudioWaveView::setData(const QByteArray &data)
{
    //先只处理8bits,unsigned

    //TODO：其他数据类型处理：16bit,float,实现数据拼接代码
    //TODO：实现升采样和降采样，即能够变换到指定的采样率
    //TODO：实现PCM音频播放功能
    //TODO：接入语音识别算法sherpa-ncnn
    //TODO：实现识别结果实时显示
    //Priority：音频播放 done->升采样降采样->数据拼接->语音识别算法->识别结果显示

    //1.波形图

    //1.1.归一化数据至[-1, 1]
    QVector<qreal> stdWaveData;
    std::transform(data.begin(), data.end(), std::back_inserter(stdWaveData), [](uint8_t currentData){
        return (currentData - 127.5) / 127.5;
    });

    //1.2.组合成点序列
    auto wavePoints = generateWavePoints(wavePointCount, waveLineSeries->pointsVector(), stdWaveData);

    //1.3.数据刷入图表
    waveLineSeries->replace(wavePoints);

    //2.频谱图

    if(wavePoints.size() == wavePointCount)
    {
        //2.0.提取数据
        QVector<qreal> fftInput;
        std::transform(wavePoints.begin(), wavePoints.end(), std::back_inserter(fftInput), [](const QPointF &point){
            return point.y();
        });

        //2.1.执行FFT
        cv::Mat_<qreal> audioData(1, fftInput.size(), fftInput.data());
        cv::Mat_<qreal> audioFFT;
        cv::dft(audioData, audioFFT);

        //2.2.导出数据
        QVector<qreal> fftData(audioFFT.cols);
        std::memcpy(fftData.data(), audioFFT.ptr<qreal>(0), audioFFT.cols * sizeof(qreal));
        auto specPoints = generateSpecPoints(specPointCount, fftData);

        //2.3.数据刷入图表
        specLineSeries->replace(specPoints);
    }
}
