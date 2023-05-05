// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class QComboBox;
class QTextEdit;
class QPushButton;
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

signals:

private:
    void initUI();
    void updateDeviceChooseBox();
    void updateDeviceParamBox(int index);

    void addMessage(const QString &message);
    void switchAudioDevice(); //true 打开，false 关闭

    QComboBox *deviceChooseBox;
    QComboBox *codecChooseBox;
    QComboBox *sampleRatesChooseBox;
    QComboBox *channelCountChooseBox;
    QComboBox *sampleTypeChooseBox;
    QComboBox *sampleSizeChooseBox;
    QComboBox *endiannessChooseBox;

    QPushButton *openAudioButton;

    QTextEdit *msgBox;
    AudioWaveView *waveView;
    AudioControl *audio;
};
