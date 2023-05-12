// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detectpage.h"
#include "ui/subwidget/imageview.h"
#include "alg/detect/yolov5s.h"
#include "utils/utils.h"
#include "utils/cameracontrol.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTabBar>
#include <QFileDialog>
#include <QTextEdit>
#include <QLabel>
#include <QTimer>

DetectPage::DetectPage(QWidget *parent)
    : QWidget(parent)
    , srcView(new ImageView)
    , dstView(new ImageView)
    , msgBox(new QTextEdit)
    , yolov5s(new Yolov5s)
    , camera(new CameraControl)
    , cameraFlushTimer(new QTimer)
{
    initUI();

    connect(cameraFlushTimer, &QTimer::timeout, [this](){
        if(!camera->isWorking())
        {
            cameraFlushTimer->stop();
            return;
        }
        openImage(camera->lastImage(), true);
    });
}

void DetectPage::initUI()
{
    //图片展示
    auto srcViewGroup = new QGroupBox(tr("原始图片"));
    auto srcViewLayer = new QHBoxLayout;
    srcViewLayer->addWidget(srcView);
    srcViewGroup->setLayout(srcViewLayer);

    auto dstViewGroup = new QGroupBox(tr("处理后图片"));
    auto dstViewLayer = new QHBoxLayout;
    dstViewLayer->addWidget(dstView);
    dstViewGroup->setLayout(dstViewLayer);

    //操作面板
    auto panelGroup = new QGroupBox(tr("操作面板"));
    auto panelLayer = new QVBoxLayout;
    auto panelSwitchWidget = new QStackedWidget;

    //图片控制面板
    auto pathEdit = new QLineEdit;
    auto pathFindButton = new QPushButton("...");
    auto openButton = new QPushButton(tr("打开图片"));

    connect(pathFindButton, &QPushButton::clicked, [pathEdit](){
        auto path = QFileDialog::getOpenFileName();
        if (!path.isEmpty())
        {
            pathEdit->setText(path);
        }
    });

    connect(openButton, &QPushButton::clicked, [this, pathEdit](){
        auto path = pathEdit->text();
        if (QFile::exists(path))
        {
            openImage(path);
        }
    });

    auto pathEnterLayer = new QHBoxLayout;
    pathEnterLayer->addWidget(pathEdit);
    pathEnterLayer->addWidget(pathFindButton);
    pathEnterLayer->addWidget(openButton);

    auto pathEnterWidget = new QWidget;
    pathEnterWidget->setLayout(pathEnterLayer);
    panelSwitchWidget->addWidget(pathEnterWidget);

    //视频控制面板
    auto videoChooseEdit = new QSpinBox;
    auto videoStatusButton = new QPushButton(tr("打开"));
    fpsLabel = new QLabel("0 fps");

    videoChooseEdit->setMinimum(0);

    connect(videoStatusButton, &QPushButton::clicked, [this, videoStatusButton, videoChooseEdit](){
        if(camera->isWorking())
        {
            if(camera->closeCamera())
            {
                videoStatusButton->setText(tr("打开"));
                addMessage(tr("摄像头已关闭"));
            }
            else
            {
                addMessage(tr("摄像头关闭失败"));
            }
        }
        else
        {
            camera->setCaptureInterval(50);
            if(camera->openCamera(videoChooseEdit->value()))
            {
                addMessage(tr("摄像头已启动"));
                videoStatusButton->setText(tr("关闭"));
                cameraFlushTimer->start(100);
            }
            else
            {
                addMessage(tr("摄像头启动失败"));
            }
        }
    });

    auto videoControlLayer = new QHBoxLayout;
    videoControlLayer->addWidget(videoChooseEdit);
    videoControlLayer->addWidget(videoStatusButton);
    videoControlLayer->addWidget(fpsLabel, 0, Qt::AlignRight);

    auto videoControlWidget = new QWidget;
    videoControlWidget->setLayout(videoControlLayer);
    panelSwitchWidget->addWidget(videoControlWidget);

    //组装tab
    auto viewChangeTab = new QTabBar;
    viewChangeTab->addTab(tr("图片测试"));
    viewChangeTab->addTab(tr("视频测试"));

    connect(viewChangeTab, &QTabBar::currentChanged, panelSwitchWidget, &QStackedWidget::setCurrentIndex);

    panelLayer->addWidget(viewChangeTab);
    panelLayer->addWidget(panelSwitchWidget, 0, Qt::AlignTop);
    panelLayer->addWidget(msgBox);
    panelGroup->setLayout(panelLayer);

    msgBox->setReadOnly(true);

    //layout组装
    auto viewLayer = new QHBoxLayout;
    viewLayer->addWidget(srcViewGroup);
    viewLayer->addWidget(dstViewGroup);

    auto allLayer = new QHBoxLayout;
    allLayer->addWidget(panelGroup, 1);
    allLayer->addLayout(viewLayer, 5);

    setLayout(allLayer);
}

void DetectPage::addMessage(const QString &msg)
{
    msgBox->insertPlainText(msg + "\n");
    msgBox->moveCursor(QTextCursor::End);
}

void DetectPage::openImage(const QString &path)
{
    openImage(QImage(path), false);
}

void DetectPage::openImage(const QImage &image, bool useFps)
{
    if(image.isNull())
    {
        return;
    }

    auto srcImage = image;
    srcView->setImage(srcImage);

    //1.执行计算
    yolov5s->setImage(srcImage);

    double timeUsed = 0;
    runWithTime([this](){
        yolov5s->analyze();
    }, &timeUsed);

    auto result = yolov5s->result();

    //2.绘制结果
    QImage imageOut(srcImage);
    QPainter painter(&imageOut);
    for(auto &eachResult : result)
    {
        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(eachResult.bbox);
        painter.drawText(eachResult.bbox.x() + 5, eachResult.bbox.y() + 14, eachResult.clsStr);
    }
    painter.end();
    dstView->setImage(imageOut);

    if(!useFps)
    {
        addMessage(tr("本轮分析用时：%1ms").arg(timeUsed));
    }
    else
    {
        double fps = 1000.0 / timeUsed;
        fpsLabel->setText(QString::number(fps, 'g', 4) + " fps");
    }
}
