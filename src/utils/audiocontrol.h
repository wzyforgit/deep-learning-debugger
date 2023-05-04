// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class AudioControl : public QObject
{
    Q_OBJECT
public:
    explicit AudioControl(QObject *parent = nullptr);

signals:

};
