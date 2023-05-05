// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiopage.h"
#include "ui/subwidget/audiowaveview.h"
#include "utils/audiocontrol.h"

#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFontMetrics>
#include <QAbstractItemView>

//基本逻辑：deviceChooseBox更新一次刷新一次下方控件

AudioPage::AudioPage(QWidget *parent)
    : QWidget(parent)
    , audio(new AudioControl(this))
{
    initUI();
    updateDeviceChooseBox();
    updateDeviceParamBox(0);
}

void AudioPage::initUI()
{
    auto panelGroup = new QGroupBox(tr("操作面板"));

    auto deviceChooseLabel = new QLabel(tr("音频输入设备"));
    deviceChooseBox = new QComboBox;
    deviceChooseBox->setMaximumWidth(150);
    connect(deviceChooseBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AudioPage::updateDeviceParamBox);

    auto codecChooseLabel = new QLabel(tr("数据编码格式"));
    codecChooseBox = new QComboBox;

    auto freqChooseLabel = new QLabel(tr("采样率"));
    sampleRatesChooseBox = new QComboBox;

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
    audioInputLayer->addWidget(sampleRatesChooseBox, 2, 1);
    audioInputLayer->addWidget(channelCountChooseLabel, 3, 0);
    audioInputLayer->addWidget(channelCountChooseBox, 3, 1);
    audioInputLayer->addWidget(sampleTypeChooseLabel, 4, 0);
    audioInputLayer->addWidget(sampleTypeChooseBox, 4, 1);
    audioInputLayer->addWidget(sampleSizeChooseLabel, 5, 0);
    audioInputLayer->addWidget(sampleSizeChooseBox, 5, 1);
    audioInputLayer->addWidget(endiannessChooseLabel, 6, 0);
    audioInputLayer->addWidget(endiannessChooseBox, 6, 1);

    openAudioButton = new QPushButton(tr("打开录音设备"));
    auto saveAudioToFile = new QPushButton(tr("保存至文件"));
    auto flushDeviceButton = new QPushButton(tr("刷新设备列表"));

    connect(openAudioButton, &QPushButton::clicked, this, &AudioPage::switchAudioDevice);

    connect(saveAudioToFile, &QPushButton::clicked, [](){
        ;
    });

    connect(flushDeviceButton, &QPushButton::clicked, [this](){
        if(audio->flushDeviceList())
        {
            updateDeviceChooseBox();
            updateDeviceParamBox(0);
        }
    });

    auto deviceOpenLayer = new QHBoxLayout;
    deviceOpenLayer->addWidget(openAudioButton);
    deviceOpenLayer->addWidget(saveAudioToFile);
    deviceOpenLayer->addWidget(flushDeviceButton);

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

void AudioPage::updateDeviceChooseBox()
{
    auto deviceList = audio->deviceList();
    deviceChooseBox->clear();
    deviceChooseBox->addItems(deviceList);

    //下拉框自适应
    QFontMetrics fontWidth(deviceChooseBox->font());

    int maxLen = 0;
    for(auto &deviceName : deviceList)
    {
        int len = fontWidth.horizontalAdvance(deviceName);
        if(maxLen < len)
        {
            maxLen = len;
        }
    }

    deviceChooseBox->view()->setMinimumWidth(maxLen * 1.1);
}

void AudioPage::updateDeviceParamBox(int index)
{
    codecChooseBox->clear();
    sampleRatesChooseBox->clear();
    channelCountChooseBox->clear();
    sampleTypeChooseBox->clear();
    sampleSizeChooseBox->clear();
    endiannessChooseBox->clear();

    if(!audio->setCurrentDevice(index))
    {
        addMessage(tr("切换设备失败"));
        return;
    }

    codecChooseBox->addItems(audio->codecs());
    sampleRatesChooseBox->addItems(audio->sampleRates());
    channelCountChooseBox->addItems(audio->channelCounts());
    sampleTypeChooseBox->addItems(audio->sampleTypes());
    sampleSizeChooseBox->addItems(audio->sampleSizes());
    endiannessChooseBox->addItems(audio->endians());
}

void AudioPage::addMessage(const QString &message)
{
    msgBox->insertPlainText(message + "\n");
}

void AudioPage::switchAudioDevice()
{
    if(audio->isWorking())
    {
        if(audio->closeAudio())
        {
            addMessage(tr("录音设备已关闭"));
            openAudioButton->setText(tr("打开录音设备"));
        }
        else
        {
            addMessage(tr("录音设备关闭失败"));
        }
    }
    else
    {
        QList<int> params;
        params << codecChooseBox->currentIndex()
               << sampleRatesChooseBox->currentIndex()
               << channelCountChooseBox->currentIndex()
               << sampleTypeChooseBox->currentIndex()
               << sampleSizeChooseBox->currentIndex()
               << endiannessChooseBox->currentIndex();
        if(audio->openAudio(params))
        {
            addMessage(tr("录音设备已打开"));
            openAudioButton->setText(tr("关闭录音设备"));
        }
        else
        {
            addMessage(tr("录音设备打开失败"));
        }
    }
}
