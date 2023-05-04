// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiopage.h"
#include "ui/subwidget/audiowaveview.h"

#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>

//基本逻辑：deviceChooseBox更新一次刷新一次下方控件

AudioPage::AudioPage(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void AudioPage::initUI()
{
    auto panelGroup = new QGroupBox(tr("操作面板"));

    auto deviceChooseLabel = new QLabel(tr("音频输入设备"));
    deviceChooseBox = new QComboBox;

    auto codecChooseLabel = new QLabel(tr("数据编码格式"));
    codecChooseBox = new QComboBox;

    auto freqChooseLabel = new QLabel(tr("采样率"));
    freqChooseBox = new QComboBox;

    auto channelCountChooseLabel = new QLabel(tr("声道数量"));
    channelCountChooseBox = new QComboBox;

    auto sampleTypeChooseLabel = new QLabel(tr("采样数据格式"));
    sampleTypeChooseBox = new QComboBox;

    auto sampleSizeChooseLabel = new QLabel(tr("采样位数"));
    sampleSizeChooseBox = new QComboBox;

    auto endiannessChooseLabel = new QLabel(tr("采样数据端序"));
    endiannessChooseBox = new QComboBox;

    auto audioInputLayer = new QGridLayout;
    audioInputLayer->addWidget(deviceChooseLabel, 0, 0);
    audioInputLayer->addWidget(deviceChooseBox, 0, 1);
    audioInputLayer->addWidget(codecChooseLabel, 1, 0);
    audioInputLayer->addWidget(codecChooseBox, 1, 1);
    audioInputLayer->addWidget(freqChooseLabel, 2, 0);
    audioInputLayer->addWidget(freqChooseBox, 2, 1);
    audioInputLayer->addWidget(channelCountChooseLabel, 3, 0);
    audioInputLayer->addWidget(channelCountChooseBox, 3, 1);
    audioInputLayer->addWidget(sampleTypeChooseLabel, 4, 0);
    audioInputLayer->addWidget(sampleTypeChooseBox, 4, 1);
    audioInputLayer->addWidget(sampleSizeChooseLabel, 5, 0);
    audioInputLayer->addWidget(sampleSizeChooseBox, 5, 1);
    audioInputLayer->addWidget(endiannessChooseLabel, 6, 0);
    audioInputLayer->addWidget(endiannessChooseBox, 6, 1);

    auto openAudioButton = new QPushButton(tr("打开录音设备"));
    auto saveAudioToFile = new QPushButton(tr("保存至文件"));
    auto saveLabel = new QLabel(tr("未开启保存"));
    auto deviceOpenLayer = new QHBoxLayout;
    deviceOpenLayer->addWidget(openAudioButton);
    deviceOpenLayer->addWidget(saveAudioToFile);
    deviceOpenLayer->addWidget(saveLabel);

    msgBox = new QTextEdit;

    auto panelLayer = new QVBoxLayout;
    panelLayer->addLayout(audioInputLayer);
    panelLayer->addLayout(deviceOpenLayer);
    panelLayer->addWidget(msgBox);
    panelGroup->setLayout(panelLayer);

    waveView = new AudioWaveView;

    auto allLayer = new QHBoxLayout;
    allLayer->addWidget(panelGroup, 1);
    allLayer->addWidget(waveView, 5);

    setLayout(allLayer);
}

void AudioPage::openAudioDevice(bool status)
{
    ;
}
