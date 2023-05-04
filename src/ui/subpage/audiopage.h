// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class QComboBox;
class QTextEdit;
class AudioWaveView;

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
    void openAudioDevice(bool status); //true 打开，false 关闭

    QComboBox *deviceChooseBox;
    QComboBox *codecChooseBox;
    QComboBox *freqChooseBox;
    QComboBox *channelCountChooseBox;
    QComboBox *sampleTypeChooseBox;
    QComboBox *sampleSizeChooseBox;
    QComboBox *endiannessChooseBox;

    QTextEdit *msgBox;
    AudioWaveView *waveView;
};
