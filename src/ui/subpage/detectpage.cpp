// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detectpage.h"
#include "ui/subwidget/imageview.h"
#include "alg/detect/yolov5s.h"
#include "utils/utils.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTabBar>
#include <QFileDialog>
#include <QTextEdit>

DetectPage::DetectPage(QWidget *parent)
    : QWidget(parent)
    , srcView(new ImageView)
    , dstView(new ImageView)
    , msgBox(new QTextEdit)
    , yolov5s(new Yolov5s)
{
    initUI();
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

    auto viewChangeTab = new QTabBar;
    viewChangeTab->addTab(tr("图片测试"));
    viewChangeTab->addTab(tr("视频测试"));

    panelLayer->addWidget(viewChangeTab);
    panelLayer->addLayout(pathEnterLayer);
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
}

void DetectPage::openImage(const QString &path)
{
    //TODO：需要改良yolov5s类的行为
    //TODO：vulkan推理结果不正确

    auto srcImage = QImage(path);
    srcView->setImage(srcImage);

    QImage imageIn(640, 640, QImage::Format_RGB888);
    imageIn.fill(0);

    QPainter painter(&imageIn);
    if(srcImage.width() > srcImage.height()) {
        srcImage = srcImage.scaledToWidth(640, Qt::SmoothTransformation);
        painter.drawImage(QPointF(0, (640 - srcImage.height()) / 2), srcImage);
    } else {
        srcImage = srcImage.scaledToHeight(640, Qt::SmoothTransformation);
        painter.drawImage(QPointF((640 - srcImage.width()) / 2, 0), srcImage);
    }
    painter.end();

    //1.执行计算
    yolov5s->setImage(imageIn);

    double timeUsed = 0;
    runWithTime([this](){
        yolov5s->analyze();
    }, &timeUsed);

    auto result = yolov5s->result();

    //2.绘制结果
    QImage imageOut(imageIn);
    painter.begin(&imageOut);
    for(auto &eachResult : result) {
        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(eachResult.bbox);
        painter.drawText(eachResult.bbox.x() + 5, eachResult.bbox.y() + 14, eachResult.clsStr);
    }
    painter.end();
    dstView->setImage(imageOut);

    addMessage(tr("本轮分析用时：%1ms").arg(timeUsed));
}
