// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiowaveview.h"

#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QLabel>
#include <QVBoxLayout>

#include <QtDebug>

#include <cmath>

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
    specYAxis->setRange(0, 100);
    specYAxis->setLabelFormat("%g");

    specChart->addAxis(specXAxis, Qt::AlignBottom);
    specChart->addAxis(specYAxis, Qt::AlignLeft);
    specChart->addSeries(specLineSeries);
    specChart->legend()->hide();
    specChart->setTitle(tr("频谱图"));

    auto specChartView = new QtCharts::QChartView;
    specChartView->setChart(specChart);

    //波形图
    waveLineSeries = new QtCharts::QLineSeries;
    waveXAxis = new QtCharts::QValueAxis;
    waveYAxis = new QtCharts::QValueAxis;
    waveChart = new QtCharts::QChart;

    waveXAxis->setTitleText(tr("时刻"));
    waveXAxis->setRange(0, 2);
    waveXAxis->setLabelFormat("%g");

    waveYAxis->setTitleText(tr("振幅"));
    //waveYAxis->setRange(-1, 1);
    waveYAxis->setRange(-2, 2);
    waveYAxis->setLabelFormat("%g");

    waveChart->addAxis(waveXAxis, Qt::AlignBottom);
    waveChart->addAxis(waveYAxis, Qt::AlignLeft);
    waveChart->addSeries(waveLineSeries);
    waveChart->legend()->hide();
    waveChart->setTitle(tr("波形图"));

    auto waveChartView = new QtCharts::QChartView;
    waveChartView->setChart(waveChart);

    auto allLayer = new QVBoxLayout;
    allLayer->addWidget(specChartView);
    allLayer->addWidget(waveChartView);

    setLayout(allLayer);
}

void AudioWaveView::setAudioParam(const QAudioFormat &format)
{
    currentFormat = format;
    //waveXAxis->setRange(0, format.sampleRate());
    waveXAxis->setRange(0, 8192);
    specXAxis->setRange(0, format.sampleRate() / 2);
}

void AudioWaveView::setData(const QByteArray &data)
{
    //先只处理8bits,unsigned

    //归一化数据至[-1, 1]
    QList<qreal> stdWaveData;
    std::transform(data.begin(), data.end(), std::back_inserter(stdWaveData), [](uint8_t currentData){
        //return (currentData - 127.5) / 127.5;
        return currentData * 10.0 / 255.0;
    });

    //组合成点序列
    //int waveFullPointCount = currentFormat.sampleRate();
    int waveFullPointCount = 8192;
    QVector<QPointF> points;
    auto oldPoints = waveLineSeries->pointsVector();
    if(oldPoints.size() < waveFullPointCount)
    {
        points = oldPoints;
    }
    else
    {
        for(int i = stdWaveData.size();i < oldPoints.size();++i)
        {
            points.push_back(QPointF(i - stdWaveData.size(), oldPoints.at(i).y()));
        }
    }
    int startSize = points.size();
    for(int i = 0;i != stdWaveData.size();++i)
    {
        points.push_back(QPointF(i + startSize, stdWaveData.at(i)));
    }

    //数据刷入图表
    waveLineSeries->replace(points);
}
