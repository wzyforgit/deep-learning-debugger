// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class QComboBox;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QFile;
class AudioWaveView;
class AudioControl;

class AudioPage : public QWidget
{
    Q_OBJECT
public:
    explicit AudioPage(QWidget *parent = nullptr);

    QString pageName() const
    {
        return tr("音频识别");
    }

private:
    void initUI();
    QWidget *initFileUI();
    QWidget *initAudioUI();

    void updateDeviceChooseBox();
    void updateDeviceParamBox(int index);
    void updateUIWhenAudioSwitch(bool open); //true:打开设备 false:关闭设备

    void addMessage(const QString &message);
    void switchAudioDevice();
    void switchAudioSave();
    void switchAudioFileOpen();

    //录音用
    QComboBox *deviceChooseBox;
    QComboBox *codecChooseBox;
    QComboBox *sampleRatesChooseBox;
    QComboBox *channelCountChooseBox;
    QComboBox *sampleTypeChooseBox;
    QComboBox *sampleSizeChooseBox;
    QComboBox *endiannessChooseBox;

    QPushButton *openAudioButton;
    QPushButton *saveAudioToFile;
    QPushButton *flushDeviceButton;

    //播放文件用
    QLineEdit *fileSampleRatesEdit;
    QComboBox *fileChannelCountChooseBox;
    QComboBox *fileSampleTypeChooseBox;
    QComboBox *fileSampleSizeChooseBox;
    QComboBox *fileEndiannessChooseBox;

    QLineEdit *fileChooseEdit;
    QComboBox *fileCodecChooseBox;
    QPushButton *openFileButton;

    //公共设施
    QTextEdit *msgBox;
    AudioWaveView *waveView;
    AudioControl *audio;

    QFile *saveFile;
    bool inSave = false;
};
