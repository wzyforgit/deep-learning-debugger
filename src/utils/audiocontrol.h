// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QAudioFormat>

class QAudioInput;
class QAudioOutput;
class AudioControlDatas;
class AudioControlDataDevice;

class AudioControl : public QObject
{
    Q_OBJECT
public:
    explicit AudioControl(QObject *parent = nullptr);
    ~AudioControl();

    //获取设备列表
    QStringList deviceList() const;

    //刷新设备列表
    bool flushDeviceList();

    //设置当前准备打开的设备
    bool setCurrentDevice(int index);

    //获取当前设备的信息
    QStringList codecs() const;        //编码格式
    QStringList sampleRates() const;   //采样率
    QStringList channelCounts() const; //通道数
    QStringList sampleTypes() const;   //采样数据类型
    QStringList sampleSizes() const;   //采样数据长度
    QStringList endians() const;       //采样数据端序

    //打开设备，参数为选择的参数值的下标，需要传入6个整数值，分别对应上面的设备信息
    bool openAudio(const QList<int> &paramIndexes);

    //打开文件，读取数据并进行播放
    bool openAudio(const QString &fileName, const QAudioFormat &format);

    //关闭设备
    bool closeAudio();

    //设备是否正在工作
    bool isWorking() const;

    //当前设置的format
    QAudioFormat format() const;

signals:
    //数据就绪
    void dataReady(const QByteArray &data);

private:
    AudioControlDatas *data;
    AudioControlDataDevice *dataDevice;
    QAudioInput *audioInput = nullptr;
    QAudioOutput *audioOutput = nullptr;
};
