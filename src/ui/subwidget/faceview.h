// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QListView>
#include <QImage>

class QStandardItemModel;

class FaceView : public QListView
{
    Q_OBJECT
public:
    explicit FaceView(QWidget *parent = nullptr);
    void setImages(const QList<QImage> &images);
    QList<int> imageChoosed() const;

private:
    QStandardItemModel *model;
};
