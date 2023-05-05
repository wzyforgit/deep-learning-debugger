// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <QAudioFormat>

namespace QtCharts {
class QLineSeries;
class QValueAxis;
class QChart;
}

class QLabel;

class AudioWaveView : public QWidget
{
    Q_OBJECT
public:
    explicit AudioWaveView(QWidget *parent = nullptr);

    void setAudioParam(const QAudioFormat &format);
    void setData(const QByteArray &data);

private:
    void initUI();

    //频谱图控件
    QtCharts::QLineSeries *specLineSeries;
    QtCharts::QValueAxis *specXAxis;
    QtCharts::QValueAxis *specYAxis;
    QtCharts::QChart *specChart;

    //波形图控件
    QtCharts::QLineSeries *waveLineSeries;
    QtCharts::QValueAxis *waveXAxis;
    QtCharts::QValueAxis *waveYAxis;
    QtCharts::QChart *waveChart;

    //format相关
    QAudioFormat currentFormat;
};
