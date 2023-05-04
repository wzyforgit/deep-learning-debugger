// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiowaveview.h"

#include <QLabel>
#include <QHBoxLayout>

AudioWaveView::AudioWaveView(QWidget *parent)
    : QWidget(parent)
{
    auto centerLabel = new QLabel(tr("这是波形显示模块"));
    auto allLayer = new QHBoxLayout;
    allLayer->addWidget(centerLabel);

    setLayout(allLayer);
}
