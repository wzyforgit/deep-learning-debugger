// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class QListView;
class QTableView;
class QTextEdit;
class QStringListModel;
class QStandardItemModel;

class ImageView;
class Cifar10;

class ClassifyPage : public QWidget
{
    Q_OBJECT
public:
    explicit ClassifyPage(QWidget *parent = nullptr);

    QString pageName() const
    {
        return tr("图片分类");
    }

signals:

private:
    void initUI();
    void startClassify(const QString &testPicDir, const QString &trainPicDir);
    void runClassify(int index);
    void showClassifyResult(const QList<QPair<QString, float> > &clsPair);
    void addMessage(const QString &message);

    QStringListModel *classifyFileModel;
    QListView *classifyFileView;
    QStandardItemModel *classifyResultModel;
    QTableView *classifyResultView;
    QTextEdit *msgBox;
    ImageView *classifyPicView;
    ImageView *trainPicView;

    QString currentTestPicDir;
    QString currentTrainPicDir;

    Cifar10 *cifar10;
};
