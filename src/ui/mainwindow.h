// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMainWindow>

//基础控件
class QTabBar;
class QStackedWidget;

//tab页面
class ClassifyPage;
class DetectPage;
class AudioPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private:
    void initTab();

    QTabBar *tabBar;
    QStackedWidget *pageWidget;

    ClassifyPage *classifyPage;
    DetectPage *detectPage;
    AudioPage *audioPage;
};
