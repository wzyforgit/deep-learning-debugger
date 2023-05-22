// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "faceview.h"

#include <QStandardItemModel>
#include <QStandardItem>

FaceView::FaceView(QWidget *parent)
    : QListView(parent)
    , model(new QStandardItemModel)
{
    setModel(model);
    setLayoutDirection(Qt::LeftToRight);
    setMovement(QListView::Static);
    setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
    setIconSize(QSize(100, 100));
    setGridSize(QSize(110, 110));
}

void FaceView::setImages(const QList<QImage> &images)
{
    model->clear();
    QList<QStandardItem*> items;
    for(int i = 0;i != images.size();++i)
    {
        auto item = new QStandardItem(QIcon(QPixmap::fromImage(images[i])), QString::number(i));
        items.push_back(item);
    }
    model->appendColumn(items);
}

QList<int> FaceView::imageChoosed() const
{
    auto indexes = selectedIndexes();
    QList<int> result;
    for(int i = 0;i != indexes.size();++i)
    {
        result.push_back(indexes[i].row());
    }
    return result;
}
