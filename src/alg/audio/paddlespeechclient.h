// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QAudioFormat>

class QTcpSocket;
class QTimer;
class QFile;

class PaddleSpeechClient : public QObject
{
    Q_OBJECT
public:
    PaddleSpeechClient(QObject *parent = nullptr);
    ~PaddleSpeechClient();

    void setCacheDir(const QString &dirPath);
    void setDataSendingInterval(int ms);
    void connectToServer(const QString &address, qint16 port);
    void startAnalyze(bool start);
    void loginToServer();
    void logoutToServer();

    void setData(const QByteArray &data);
    void setAudioFormat(const QAudioFormat &format);
    void sendDataToServer();
    void sendJsonRequest(const QString &data);

signals:
    void connectStateChanged(bool state);
    void analyzeResultReady(const QString &data);
    void recvError(const QString &code, const QString &reason);

private:
    QString createLoginRequest();
    QString createAsrRequest();
    void parseRecvJson(const QByteArray &data);

    QString cacheDir;
    QString token;
    QTcpSocket *socket;
    QTimer *sendTimer;
    QFile *cacheFile;

    QAudioFormat audioFormat;
    QByteArray cachedData;

    int sendingInterval = 5000;
    quint16 remainingDataLength = 0;
    bool isConnected = false;
};
