// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "classifypage.h"
#include "ui/subwidget/imageview.h"
#include "alg/classify/cifar10.h"
#include "utils/utils.h"

#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QStringListModel>
#include <QListView>
#include <QStandardItemModel>
#include <QTableView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QHeaderView>

#include <QtDebug>

ClassifyPage::ClassifyPage(QWidget *parent)
    : QWidget(parent)
    , cifar10(new Cifar10)
{
    initUI();
}

void ClassifyPage::initUI()
{
    //文件目录选择
    auto classifyFileDirChooseLabel = new QLabel(tr("测试图片目录："));
    auto classifyFileDirChooseEdit = new QLineEdit;
    auto classifyFileDirChooseButton = new QPushButton(tr("..."));
    connect(classifyFileDirChooseButton, &QPushButton::clicked, [classifyFileDirChooseEdit](){
        auto dir = QFileDialog::getExistingDirectory();
        if(!dir.isEmpty())
        {
            classifyFileDirChooseEdit->setText(dir);
        }
    });

    auto classifyFileDirChooseLayer = new QHBoxLayout;
    classifyFileDirChooseLayer->addWidget(classifyFileDirChooseLabel);
    classifyFileDirChooseLayer->addWidget(classifyFileDirChooseEdit);
    classifyFileDirChooseLayer->addWidget(classifyFileDirChooseButton);

    auto classifyTrainDirChooseLabel = new QLabel(tr("训练图片目录："));
    auto classifyTrainDirChooseEdit = new QLineEdit;
    auto classifyTrainDirChooseButton = new QPushButton(tr("..."));
    connect(classifyTrainDirChooseButton, &QPushButton::clicked, [classifyTrainDirChooseEdit](){
        auto dir = QFileDialog::getExistingDirectory();
        if(!dir.isEmpty())
        {
            classifyTrainDirChooseEdit->setText(dir);
        }
    });

    auto classifyFileTrainChooseLayer = new QHBoxLayout;
    classifyFileTrainChooseLayer->addWidget(classifyTrainDirChooseLabel);
    classifyFileTrainChooseLayer->addWidget(classifyTrainDirChooseEdit);
    classifyFileTrainChooseLayer->addWidget(classifyTrainDirChooseButton);

    auto startClassifyButton = new QPushButton(tr("开始分类测试"));
    connect(startClassifyButton, &QPushButton::clicked, [classifyFileDirChooseEdit, classifyTrainDirChooseEdit, this](){
        auto testDir = classifyFileDirChooseEdit->text();
        auto trainDir = classifyTrainDirChooseEdit->text();
        QFileInfo testDirInfo(testDir);
        QFileInfo trainDirInfo(trainDir);
        if(testDirInfo.isDir() && trainDirInfo.isDir())
        {
            startClassify(testDir, trainDir);
        }
    });

    //分类图片列表
    classifyFileModel = new QStringListModel;
    classifyFileView = new QListView;
    classifyFileView->setModel(classifyFileModel);
    classifyFileView->setEditTriggers(QListView::NoEditTriggers);
    connect(classifyFileView, &QListView::pressed, [this](const QModelIndex &index){
        runClassify(index.row());
    });

    //分类结果列表
    classifyResultModel = new QStandardItemModel(5, 2);
    classifyResultModel->setHorizontalHeaderLabels({tr("预测项"), tr("概率")});
    classifyResultView = new QTableView;
    classifyResultView->setModel(classifyResultModel);
    classifyResultView->setEditTriggers(QTableView::NoEditTriggers);
    classifyResultView->setSelectionBehavior(QTableView::SelectRows);
    classifyResultView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    classifyResultView->verticalHeader()->hide();
    classifyResultView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(classifyResultView, &QTableView::pressed, [this](const QModelIndex &index){
        auto label = classifyResultModel->data(classifyResultModel->index(index.row(), 0)).toString();
        QDir dir(currentTrainPicDir + label);
        if(!dir.exists())
        {
            addMessage(tr("训练数据文件夹不存在"));
            return;
        }
        auto picList = dir.entryList({"*.jpg", "*.jpeg", "*.png", "*.bmp"});
        std::transform(picList.begin(), picList.end(), picList.begin(), [dir](const QString &fileName){
            return dir.path() + '/' + fileName;
        });
        trainPicView->setImages(picList);
    });

    //消息框
    msgBox = new QTextEdit;
    msgBox->setReadOnly(true);

    //整合操作面板
    auto panelLayer = new QVBoxLayout;
    panelLayer->addLayout(classifyFileDirChooseLayer);
    panelLayer->addLayout(classifyFileTrainChooseLayer);
    panelLayer->addWidget(startClassifyButton);
    panelLayer->addWidget(classifyFileView);
    panelLayer->addWidget(classifyResultView);
    panelLayer->addWidget(msgBox);

    auto panelGroupBox = new QGroupBox(tr("操作面板"));
    panelGroupBox->setLayout(panelLayer);

    //图片显示区域
    classifyPicView = new ImageView;
    auto classifyPicLayer = new QHBoxLayout;
    classifyPicLayer->addWidget(classifyPicView);
    auto classifyPicGroupBox = new QGroupBox(tr("测试图片"));
    classifyPicGroupBox->setLayout(classifyPicLayer);

    trainPicView = new ImageView;
    auto trainPicLayer = new QHBoxLayout;
    trainPicLayer->addWidget(trainPicView);
    auto trainPicGroupBox = new QGroupBox(tr("训练图片"));
    trainPicGroupBox->setLayout(trainPicLayer);

    auto viewLayer = new QHBoxLayout;
    viewLayer->addWidget(classifyPicGroupBox);
    viewLayer->addWidget(trainPicGroupBox);

    //整合全部控件
    auto allLayer = new QHBoxLayout;
    allLayer->addWidget(panelGroupBox, 1);
    allLayer->addLayout(viewLayer, 5);
    setLayout(allLayer);
}

void ClassifyPage::startClassify(const QString &testPicDir, const QString &trainPicDir)
{
    //1.记录需要探测的目录
    currentTestPicDir = testPicDir;
    if(!currentTestPicDir.endsWith("/"))
    {
        currentTestPicDir += "/";
    }

    currentTrainPicDir = trainPicDir;
    if(!currentTrainPicDir.endsWith("/"))
    {
        currentTrainPicDir += "/";
    }

    //2.设置测试图片列表
    QDir dir(currentTestPicDir);
    auto pics = dir.entryList({"*.jpg", "*.jpeg", "*.png", "*.bmp"});
    if(pics.isEmpty())
    {
        addMessage(tr("未发现测试图片，仅支持jpg, png, bmp格式图片"));
        return;
    }
    classifyFileModel->setStringList(pics);

    //3.设置第一个图片的推理
    classifyFileView->setCurrentIndex(classifyFileModel->index(0));
    runClassify(0);
}

void ClassifyPage::addMessage(const QString &message)
{
    msgBox->moveCursor(QTextCursor::End);
    msgBox->insertPlainText(message + "\n");
    msgBox->moveCursor(QTextCursor::End);
}

void ClassifyPage::runClassify(int index)
{
    auto pics = classifyFileModel->stringList();
    if(index < 0 || index >= pics.size())
    {
        return;
    }

    //获取当前图片
    auto currentPicturePath = currentTestPicDir + pics.at(index);

    //读取待分析图片，刷入原始图片显示框
    QImage originPic(currentPicturePath);
    classifyPicView->setImage(originPic);

    //执行分析过程
    cifar10->setImage(originPic);

    double timeUsed = 0;
    runWithTime([this](){
        cifar10->analyze();
    }, &timeUsed);

    addMessage(tr("本次分析耗时: %1ms").arg(timeUsed));

    auto classifyResult = cifar10->result();
    showClassifyResult(classifyResult.clsPair);
}

void ClassifyPage::showClassifyResult(const QList<QPair<QString, float>> &clsPair)
{
    for(int i = 0;i != 5 && i < clsPair.size();++i)
    {
        classifyResultModel->setData(classifyResultModel->index(i, 0), clsPair[i].first);
        classifyResultModel->setData(classifyResultModel->index(i, 1), clsPair[i].second);
    }
}
