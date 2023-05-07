// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiopage.h"
#include "ui/subwidget/audiowaveview.h"
#include "utils/audiocontrol.h"

#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFontMetrics>
#include <QAbstractItemView>
#include <QTabBar>
#include <QStackedWidget>
#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>

#include <QtDebug>

//基本逻辑：deviceChooseBox更新一次刷新一次下方控件

AudioPage::AudioPage(QWidget *parent)
    : QWidget(parent)
    , audio(new AudioControl(this))
    , saveFile(new QFile)
{
    initUI();
    updateDeviceChooseBox();
    updateDeviceParamBox(0);
}

QWidget* AudioPage::initAudioUI()
{
    auto deviceChooseLabel = new QLabel(tr("音频输入设备"));
    deviceChooseBox = new QComboBox;
    deviceChooseBox->setMaximumWidth(150);
    connect(deviceChooseBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AudioPage::updateDeviceParamBox);

    auto codecChooseLabel = new QLabel(tr("数据编码格式"));
    codecChooseBox = new QComboBox;

    auto sampleRatesChooseLabel = new QLabel(tr("采样率"));
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
    audioInputLayer->addWidget(sampleRatesChooseLabel, 2, 0);
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
    saveAudioToFile = new QPushButton(tr("保存至文件"));
    flushDeviceButton = new QPushButton(tr("刷新设备列表"));

    connect(openAudioButton, &QPushButton::clicked, this, &AudioPage::switchAudioDevice);

    saveAudioToFile->setEnabled(false);
    connect(saveAudioToFile, &QPushButton::clicked, this, &AudioPage::switchAudioSave);

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

    auto audioLayer = new QVBoxLayout;
    audioLayer->addLayout(audioInputLayer);
    audioLayer->addLayout(deviceOpenLayer);

    auto result = new QWidget;
    result->setLayout(audioLayer);
    return result;
}

QWidget* AudioPage::initFileUI()
{
    auto fileCodecChooseLabel = new QLabel(tr("数据编码格式"));
    fileCodecChooseBox = new QComboBox;
    fileCodecChooseBox->addItem("audio/pcm");

    auto fileSampleRatesLabel = new QLabel(tr("采样率"));
    fileSampleRatesEdit = new QLineEdit;
    fileSampleRatesEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+")));

    auto fileChannelCountChooseLabel = new QLabel(tr("声道数量"));
    fileChannelCountChooseBox = new QComboBox;
    fileChannelCountChooseBox->addItem("1");

    auto fileSampleTypeChooseLabel = new QLabel(tr("采样数据格式"));
    fileSampleTypeChooseBox = new QComboBox;
    fileSampleTypeChooseBox->addItems({"SignedInt", "UnSignedInt", "Float"});

    auto fileSampleSizeChooseLabel = new QLabel(tr("采样位数"));
    fileSampleSizeChooseBox = new QComboBox;
    fileSampleSizeChooseBox->addItems({"8", "16", "32"});

    auto fileEndiannessChooseLabel = new QLabel(tr("采样数据端序"));
    fileEndiannessChooseBox = new QComboBox;
    fileEndiannessChooseBox->addItems({"BigEndian", "LittleEndian"});

    auto audioInputLayer = new QGridLayout;
    audioInputLayer->addWidget(fileCodecChooseLabel, 0, 0);
    audioInputLayer->addWidget(fileCodecChooseBox, 0, 1);
    audioInputLayer->addWidget(fileSampleRatesLabel, 1, 0);
    audioInputLayer->addWidget(fileSampleRatesEdit, 1, 1, Qt::AlignRight);
    audioInputLayer->addWidget(fileChannelCountChooseLabel, 2, 0);
    audioInputLayer->addWidget(fileChannelCountChooseBox, 2, 1);
    audioInputLayer->addWidget(fileSampleTypeChooseLabel, 3, 0);
    audioInputLayer->addWidget(fileSampleTypeChooseBox, 3, 1);
    audioInputLayer->addWidget(fileSampleSizeChooseLabel, 4, 0);
    audioInputLayer->addWidget(fileSampleSizeChooseBox, 4, 1);
    audioInputLayer->addWidget(fileEndiannessChooseLabel, 5, 0);
    audioInputLayer->addWidget(fileEndiannessChooseBox, 5, 1);
    fileSampleRatesEdit->setFixedWidth(150);

    fileChooseEdit = new QLineEdit;
    auto fileChooseButton = new QPushButton("...");
    connect(fileChooseButton, &QPushButton::clicked, [this](){
        auto filePath = QFileDialog::getOpenFileName();
        if(!filePath.isEmpty())
        {
            fileChooseEdit->setText(filePath);
        }
    });

    openFileButton = new QPushButton(tr("开始播放"));
    connect(openFileButton, &QPushButton::clicked, this, &AudioPage::switchAudioFileOpen);

    auto fileChooseLayer = new QHBoxLayout;
    fileChooseLayer->addWidget(fileChooseEdit);
    fileChooseLayer->addWidget(fileChooseButton);
    //fileChooseLayer->addWidget(openFileButton);

    auto audioLayer = new QVBoxLayout;
    audioLayer->addLayout(audioInputLayer);
    audioLayer->addLayout(fileChooseLayer);
    audioLayer->addWidget(openFileButton);

    auto result = new QWidget;
    result->setLayout(audioLayer);
    return result;
}

void AudioPage::initUI()
{
    auto panelGroup = new QGroupBox(tr("操作面板"));

    msgBox = new QTextEdit;
    msgBox->setReadOnly(true);

    auto viewStack = new QStackedWidget;
    viewStack->addWidget(initFileUI());
    viewStack->addWidget(initAudioUI());

    auto viewChangeTab = new QTabBar;
    viewChangeTab->addTab(tr("播放文件"));
    viewChangeTab->addTab(tr("实时音频"));
    connect(viewChangeTab, &QTabBar::currentChanged, viewStack, &QStackedWidget::setCurrentIndex);

    auto panelLayer = new QVBoxLayout;
    panelLayer->addWidget(viewChangeTab);
    panelLayer->addWidget(viewStack, 0, Qt::AlignTop);
    panelLayer->addWidget(msgBox);
    panelGroup->setLayout(panelLayer);

    waveView = new AudioWaveView;
    connect(audio, &AudioControl::dataReady, waveView, &AudioWaveView::setData);

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
            if(inSave)
            {
                switchAudioSave();
            }
            updateUIWhenAudioSwitch(false);
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
            waveView->setAudioParam(audio->format());
            updateUIWhenAudioSwitch(true);
        }
        else
        {
            addMessage(tr("录音设备打开失败"));
        }
    }
}

void AudioPage::switchAudioSave()
{
    if(inSave)
    {
        disconnect(audio, &AudioControl::dataReady, saveFile, QOverload<const QByteArray &>::of(&QFile::write));
        saveFile->close();
        addMessage(tr("已停止保存录音"));
        saveAudioToFile->setText(tr("保存至文件"));
        inSave = false;
    }
    else
    {
        QString preFileName = QString("%1_%2c_%3_%4_%5.pcm").arg(sampleRatesChooseBox->currentText())
                                                            .arg(channelCountChooseBox->currentText())
                                                            .arg(sampleTypeChooseBox->currentText())
                                                            .arg(sampleSizeChooseBox->currentText())
                                                            .arg(endiannessChooseBox->currentText());
        auto fullPath = QFileDialog::getSaveFileName(nullptr, tr("保存音频文件"), preFileName);
        if(!fullPath.isEmpty())
        {
            saveFile->setFileName(fullPath);
            saveFile->open(QIODevice::WriteOnly);
            connect(audio, &AudioControl::dataReady, saveFile, QOverload<const QByteArray &>::of(&QFile::write));
            addMessage(tr("开始保存录音至文件"));
            saveAudioToFile->setText(tr("停止保存"));
            inSave = true;
        }
    }
}

void AudioPage::updateUIWhenAudioSwitch(bool open)
{
    if(open)
    {
        deviceChooseBox->setEnabled(false);
        codecChooseBox->setEnabled(false);
        sampleRatesChooseBox->setEnabled(false);
        channelCountChooseBox->setEnabled(false);
        sampleTypeChooseBox->setEnabled(false);
        sampleSizeChooseBox->setEnabled(false);
        endiannessChooseBox->setEnabled(false);
        flushDeviceButton->setEnabled(false);
        saveAudioToFile->setEnabled(true);
    }
    else
    {
        deviceChooseBox->setEnabled(true);
        codecChooseBox->setEnabled(true);
        sampleRatesChooseBox->setEnabled(true);
        channelCountChooseBox->setEnabled(true);
        sampleTypeChooseBox->setEnabled(true);
        sampleSizeChooseBox->setEnabled(true);
        endiannessChooseBox->setEnabled(true);
        flushDeviceButton->setEnabled(true);
        saveAudioToFile->setEnabled(false);
    }
}

void AudioPage::switchAudioFileOpen()
{
    if(audio->isWorking())
    {
        if(audio->closeAudio())
        {
            addMessage(tr("播放设备已关闭"));
            openFileButton->setText(tr("开始播放"));
        }
        else
        {
            addMessage(tr("播放设备关闭失败"));
        }
    }
    else
    {
        auto fileName = fileChooseEdit->text();
        if(!QFile::exists(fileName))
        {
            addMessage(tr("PCM文件不存在"));
            return;
        }

        auto sampleRate = fileSampleRatesEdit->text().toInt();
        if(sampleRate <= 0)
        {
            addMessage(tr("采样率设置错误"));
            return;
        }

        QAudioFormat format;
        format.setCodec(fileCodecChooseBox->currentText());
        format.setSampleRate(sampleRate);
        format.setChannelCount(fileChannelCountChooseBox->currentText().toInt());
        format.setSampleType(static_cast<QAudioFormat::SampleType>(fileSampleTypeChooseBox->currentIndex() + 1));
        format.setSampleSize(fileSampleSizeChooseBox->currentText().toInt());
        format.setByteOrder(static_cast<QAudioFormat::Endian>(fileEndiannessChooseBox->currentIndex()));

        if(audio->openAudio(fileName, format))
        {
            waveView->setAudioParam(format);
            addMessage(tr("播放设备已打开"));
            openFileButton->setText(tr("停止播放"));
        }
        else
        {
            addMessage(tr("播放设备打开失败"));
        }
    }
}
