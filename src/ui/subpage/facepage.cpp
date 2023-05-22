// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "facepage.h"
#include "ui/subwidget/imageview.h"
#include "ui/subwidget/faceview.h"
#include "utils/utils.h"
#include "utils/cameracontrol.h"
#include "alg/face/retinaface.h"
#include "alg/face/facealign.h"
#include "alg/face/arcface.h"

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
#include <QListView>

FacePage::FacePage(QWidget *parent)
    : QWidget(parent)
    , srcView(new ImageView)
    , dstView(new ImageView)
    , msgBox(new QTextEdit)
    , camera(new CameraControl)
    , cameraFlushTimer(new QTimer)
    , retinaFace(new RetinaFace)
    , faceAlign(new FaceAlign)
    , arcFace(new ArcFace)
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

void FacePage::initUI()
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

    //人脸识别面板
    auto captureFaceButton = new QPushButton(tr("截取人脸"));
    connect(captureFaceButton, &QPushButton::clicked, [this](){
        faceChooseView->setImages(alignFaces);
    });

    auto recFaceButton = new QPushButton(tr("比对选中的人脸"));
    connect(recFaceButton, &QPushButton::clicked, this, &FacePage::recFaces);

    auto recPanel = new QHBoxLayout;
    recPanel->addWidget(captureFaceButton);
    recPanel->addWidget(recFaceButton);

    //人脸选择面板
    faceChooseView = new FaceView;

    //组装tab
    auto viewChangeTab = new QTabBar;
    viewChangeTab->addTab(tr("图片测试"));
    viewChangeTab->addTab(tr("视频测试"));

    connect(viewChangeTab, &QTabBar::currentChanged, panelSwitchWidget, &QStackedWidget::setCurrentIndex);

    panelLayer->addWidget(viewChangeTab);
    panelLayer->addWidget(panelSwitchWidget, 0, Qt::AlignTop);
    panelLayer->addLayout(recPanel);
    panelLayer->addWidget(faceChooseView);
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

void FacePage::addMessage(const QString &msg)
{
    msgBox->moveCursor(QTextCursor::End);
    msgBox->insertPlainText(msg + "\n");
    msgBox->moveCursor(QTextCursor::End);
}

void FacePage::openImage(const QString &path)
{
    openImage(QImage(path), false);
}

void FacePage::openImage(const QImage &image, bool useFps)
{
    if(image.isNull())
    {
        return;
    }

    auto srcImage = image;
    srcView->setImage(srcImage);

    //执行计算
    retinaFace->setImage(srcImage);
    double timeUsed = 0;
    runWithTime([this](){
        retinaFace->analyze();
    }, &timeUsed);

    auto result = retinaFace->result();
    faceAlign->setImage(srcImage);
    alignFaces.clear();

    //绘制结果
    QImage imageOut(srcImage);
    QPainter painter(&imageOut);
    for(auto &eachResult : result)
    {
        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(eachResult.bbox);

        for(int i = 0;i != 10;i += 2)
        {
            painter.drawPoint(QPointF(eachResult.landmark[i], eachResult.landmark[i + 1]));
        }

        //保存人脸
        faceAlign->analyze(eachResult.landmark);
        alignFaces.push_back(faceAlign->result());
    }
    painter.end();
    dstView->setImage(imageOut);

    //显示用时情况
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

void FacePage::recFaces()
{
    auto chooseIndexes = faceChooseView->imageChoosed();
    QList<QImage> images;
    for(auto &eachIndex : chooseIndexes)
    {
        images.push_back(alignFaces.at(eachIndex));
    }

    QList<QVector<float>> feats;
    double timeUsed;
    runWithTime([this, images, &feats](){
        for(auto &eachImage : images)
        {
            arcFace->setImage(eachImage);
            arcFace->analyze();
            feats.push_back(arcFace->result());
        }
    }, &timeUsed);

    if(images.size() > 1)
    {
        auto sim = ArcFace::compare(feats[0], feats[1]);
        addMessage(tr("第%1和第%2的相似度为: %3").arg(chooseIndexes[0])
                                                   .arg(chooseIndexes[1])
                                                   .arg(sim));
    }
    addMessage(tr("本次分析耗时: %1ms").arg(timeUsed));
}
