// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiocontrol.h"

#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioInput>

#include <QtDebug>

class AudioControlDatas
{
public:
    //全局设备信息
    QList<QAudioDeviceInfo> deviceInfos;
    QStringList deviceNames;

    //选中的设备信息
    QAudioDeviceInfo info;
    QStringList codecs;
    QList<int> sampleRates;
    QList<int> channelCounts;
    QList<QAudioFormat::SampleType> sampleTypes;
    QList<int> sampleSizes;
    QList<QAudioFormat::Endian> endians;

    //已设置的数据
    QAudioFormat format;
};

class AudioControlDataDevice : public QIODevice
{
    Q_OBJECT

public:
    AudioControlDataDevice(QObject *parent = nullptr);

signals:
    void dataReady(const QByteArray &data);

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
};

AudioControlDataDevice::AudioControlDataDevice(QObject *parent)
    : QIODevice(parent)
{
}

qint64 AudioControlDataDevice::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return -1;
}

qint64 AudioControlDataDevice::writeData(const char *data, qint64 len)
{
    QByteArray newData(data, len);
    emit dataReady(newData);
    return len;
}

AudioControl::AudioControl(QObject *parent)
    : QObject(parent)
    , data(new AudioControlDatas)
    , dataDevice(new AudioControlDataDevice)
{
    flushDeviceList();

    connect(dataDevice, &AudioControlDataDevice::dataReady, this, &AudioControl::dataReady);
}

AudioControl::~AudioControl()
{
    delete data;

    if(audioInput != nullptr)
    {
        audioInput->stop();
        audioInput->deleteLater();
    }

    dataDevice->close();
    dataDevice->deleteLater();
}

bool AudioControl::flushDeviceList()
{
    if(isWorking())
    {
        return false;
    }

    data->deviceInfos = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    data->deviceNames.clear();
    for(auto &device : data->deviceInfos)
    {
        data->deviceNames.push_back(device.deviceName());
    }

    setCurrentDevice(0);

    return true;
}

QStringList AudioControl::deviceList() const
{
    return data->deviceNames;
}

bool AudioControl::setCurrentDevice(int index)
{
    if(index < 0 || index >= data->deviceInfos.size() || isWorking())
    {
        return false;
    }

    auto currentDevice = data->deviceInfos.at(index);
    data->info = currentDevice;
    data->codecs = currentDevice.supportedCodecs();
    data->sampleRates = currentDevice.supportedSampleRates();
    data->channelCounts = currentDevice.supportedChannelCounts();
    data->sampleTypes = currentDevice.supportedSampleTypes();
    data->sampleSizes = currentDevice.supportedSampleSizes();
    data->endians = currentDevice.supportedByteOrders();

    return true;
}

QStringList AudioControl::codecs() const
{
    return data->codecs;
}

QStringList AudioControl::sampleRates() const
{
    QStringList results;
    for(int rate : data->sampleRates)
    {
        results.push_back(QString::number(rate));
    }
    return results;
}

QStringList AudioControl::channelCounts() const
{
    QStringList results;
    for(int channelCount : data->channelCounts)
    {
        results.push_back(QString::number(channelCount));
    }
    return results;
}

QStringList AudioControl::sampleTypes() const
{
    QStringList results;
    for(auto &type : data->sampleTypes)
    {
        switch (type)
        {
        default:
            break;
        case QAudioFormat::Unknown:
            results.push_back("Unknown");
            break;
        case QAudioFormat::SignedInt:
            results.push_back("SignedInt");
            break;
        case QAudioFormat::UnSignedInt:
            results.push_back("UnSignedInt");
            break;
        case QAudioFormat::Float:
            results.push_back("Float");
            break;
        }
    }
    return results;
}

QStringList AudioControl::sampleSizes() const
{
    QStringList results;
    for(int size : data->sampleSizes)
    {
        results.push_back(QString::number(size));
    }
    return results;
}

QStringList AudioControl::endians() const
{
    QStringList results;
    for(auto &endian : data->endians)
    {
        switch (endian)
        {
        default:
            break;
        case QAudioFormat::BigEndian:
            results.push_back("BigEndian");
            break;
        case QAudioFormat::LittleEndian:
            results.push_back("LittleEndian");
            break;
        }
    }
    return results;
}

bool AudioControl::openAudio(const QList<int> &paramIndexes)
{
    if(paramIndexes.size() != 6 || audioInput != nullptr)
    {
        return false;
    }

    QAudioFormat format;
    format.setCodec(data->codecs.at(paramIndexes[0]));
    format.setSampleRate(data->sampleRates.at(paramIndexes[1]));
    format.setChannelCount(data->channelCounts.at(paramIndexes[2]));
    format.setSampleType(data->sampleTypes.at(paramIndexes[3]));
    format.setSampleSize(data->sampleSizes.at(paramIndexes[4]));
    format.setByteOrder(data->endians.at(paramIndexes[5]));

    if(!data->info.isFormatSupported(format))
    {
        return false;
    }
    data->format = format;

    audioInput = new QAudioInput(data->info, format);
    audioInput->setBufferSize(format.sampleRate() / 4 * format.sampleSize() / 8);
    dataDevice->open(QIODevice::WriteOnly);
    audioInput->start(dataDevice);

    return true;
}

bool AudioControl::closeAudio()
{
    if(audioInput != nullptr)
    {
        audioInput->stop();
        audioInput->deleteLater();
        audioInput = nullptr;
    }

    dataDevice->close();
    return true;
}

bool AudioControl::isWorking() const
{
    return audioInput != nullptr;
}

QAudioFormat AudioControl::format() const
{
    return data->format;
}

#include "audiocontrol.moc"
