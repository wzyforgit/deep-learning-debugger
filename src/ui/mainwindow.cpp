// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "ui/subpage/classifypage.h"
#include "ui/subpage/detectpage.h"
#include "ui/subpage/audiopage.h"
#include "ui/subpage/facepage.h"

#include <QTabBar>
#include <QStackedWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , tabBar(new QTabBar)
    , pageWidget(new QStackedWidget)
    , classifyPage(new ClassifyPage)
    , detectPage(new DetectPage)
    , audioPage(new AudioPage)
    , facePage(new FacePage)
{
    auto allLayer = new QVBoxLayout;
    allLayer->addWidget(tabBar);
    allLayer->addWidget(pageWidget);

    initTab();
    connect(tabBar, &QTabBar::currentChanged, pageWidget, &QStackedWidget::setCurrentIndex);

    auto center = new QWidget;
    center->setLayout(allLayer);
    setCentralWidget(center);
    resize(640, 480);
}

void MainWindow::initTab()
{
    //图片分类
    tabBar->addTab(classifyPage->pageName());
    pageWidget->addWidget(classifyPage);

    //目标检测
    tabBar->addTab(detectPage->pageName());
    pageWidget->addWidget(detectPage);

    //人脸识别
    tabBar->addTab(facePage->pageName());
    pageWidget->addWidget(facePage);

    //音频识别
    tabBar->addTab(audioPage->pageName());
    pageWidget->addWidget(audioPage);
}
