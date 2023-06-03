// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "paddlespeechclient.h"
#include "utils/utils.h"

#include <QTcpSocket>
#include <QTimer>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>

PaddleSpeechClient::PaddleSpeechClient(QObject *parent)
    : QObject(parent)
    , socket(new QTcpSocket(this))
    , sendTimer(new QTimer)
    , cacheFile(new QFile)
{
    connect(socket, &QTcpSocket::connected, this, &PaddleSpeechClient::loginToServer);
    connect(socket, &QTcpSocket::disconnected, this, &PaddleSpeechClient::logoutToServer);
    connect(socket, &QTcpSocket::readyRead, [this](){
        parseRecvJson(socket->readAll());
    });
    connect(sendTimer, &QTimer::timeout, this, &PaddleSpeechClient::sendDataToServer);
}

PaddleSpeechClient::~PaddleSpeechClient()
{
    socket->close();
}

void PaddleSpeechClient::setCacheDir(const QString &dirPath)
{
    cacheDir = dirPath;
    if(!cacheDir.endsWith("/"))
    {
        cacheDir += "/";
    }
}

void PaddleSpeechClient::setDataSendingInterval(int ms)
{
    if(ms > 0)
    {
        sendingInterval = ms;
    }
}

void PaddleSpeechClient::startAnalyze(bool start)
{
    if(start)
    {
        sendTimer->start(sendingInterval);
    }
    else
    {
        sendTimer->stop();
    }
}

void PaddleSpeechClient::connectToServer(const QString &address, qint16 port)
{
    socket->connectToHost(address, port);
}

void PaddleSpeechClient::setData(const QByteArray &data)
{
    if(!isConnected || !cacheFile->isOpen())
    {
        return;
    }

    cacheFile->write(data);
}

void PaddleSpeechClient::loginToServer()
{
    sendJsonRequest(createLoginRequest());
}

void PaddleSpeechClient::logoutToServer()
{
    isConnected = false;

    if(cacheFile->exists())
    {
        cacheFile->close();
        cacheFile->remove();
    }
}

void PaddleSpeechClient::setAudioFormat(const QAudioFormat &format)
{
    audioFormat = format;
}

QString PaddleSpeechClient::createLoginRequest()
{
    return "{ \"key\" : \"123321\", \"pwd\" : \"321123\" }";
}

QString PaddleSpeechClient::createAsrRequest()
{
    const QString fileToken("12345");
    cacheFile->close();
    auto waveFileName = cacheDir + QString("%1_%2.wav").arg(token).arg(fileToken);
    cacheFile->copy(waveFileName);
    cacheFile->waitForBytesWritten(-1);
    createPaddleSpeechWaveFile(waveFileName, audioFormat);
    cacheFile->open(QIODevice::WriteOnly);

    return QString("{ \"token\" : \"%1\", \"task\" : \"asr_%2\", \"file_token\" : \"%3\" }")
            .arg(token).arg(123123).arg(fileToken);
}

void PaddleSpeechClient::parseRecvJson(const QByteArray &data)
{
    QByteArray recvData;
    if(remainingDataLength == 0 && cachedData.size() == 0)
    {
        remainingDataLength = (static_cast<quint8>(data[0]) << 8) | static_cast<quint8>(data[1]);
        cachedData = data.right(data.size() - 2);
        if(cachedData.size() == remainingDataLength)
        {
            recvData = cachedData;
            cachedData.clear();
            remainingDataLength = 0;
        }
        else
        {
            remainingDataLength -= cachedData.size();
        }
    }
    else if(remainingDataLength != 0 && cachedData.size() != 0)
    {
        if(data.size() < remainingDataLength)
        {
            remainingDataLength -= data.size();
            cachedData += data;
        }
        else if(data.size() == remainingDataLength)
        {
            remainingDataLength = 0;
            recvData = cachedData + data;
            cachedData.clear();
        }
        else
        {
            qDebug() << "Error: Did not take this into account";
            qDebug() << "remainingDataLength:" << remainingDataLength;
            qDebug() << "cachedData.size" << cachedData.size();
            qDebug() << "data.size" << data.size();
            std::abort();
        }
    }
    else
    {
        std::abort();
    }

    if(recvData.isEmpty())
    {
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(recvData);
    auto jsonObj = jsonDoc.object();

    if(!isConnected)
    {
        auto keys = jsonObj.keys();
        if(keys.size() == 2 && keys.contains("token"))
        {
            token = jsonObj["token"].toString();
            isConnected = true;
            cacheFile->setFileName(cacheDir + QString("paddlespeech_cache.wav"));
            cacheFile->open(QIODevice::WriteOnly);
        }
        else
        {
            emit recvError(jsonObj["status"].toString(), jsonObj["reason"].toString());
        }
    }
    else
    {
        auto keys = jsonObj.keys();
        if(keys.size() != 2)
        {
            auto task = jsonObj["task"].toString().split("_")[0];
            if(task == "asr") //暂时只解析ASR数据
            {
                auto result = jsonObj["result"].toString();
                emit analyzeResultReady(result);
            }
        }
        else
        {
            emit recvError(jsonObj["status"].toString(), jsonObj["reason"].toString());
        }
    }
}

void PaddleSpeechClient::sendDataToServer()
{
    sendJsonRequest(createAsrRequest());
}

void PaddleSpeechClient::sendJsonRequest(const QString &data)
{
    qint16 dataLen = data.size();
    QByteArray lenData;
    lenData.append(static_cast<quint8>(dataLen >> 8));
    lenData.append(static_cast<quint8>(dataLen & 0xFF));

    socket->write(lenData + data.toUtf8());
}
