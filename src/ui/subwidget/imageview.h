// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

//改编自：https://github.com/linuxdeepin/deepin-ocr/blob/master/src/view/imageview.h
//原版权所有者：UnionTech Software Technology Co., Ltd.

#pragma once

#include <QGraphicsView>

class QGraphicsPixmapItem;

class ImageView : public QGraphicsView
{
    Q_OBJECT
public:
    ImageView(QWidget *parent = nullptr);
    ~ImageView() = default;
    //设置图片
    void setImage(const QImage &image);
    //用于鼠标滚轮滑动
    qreal windowRelativeScale() const;
    qreal imageRelativeScale() const;
    void scaleAtPoint(QPointF pos, qreal factor);
    void setScaleValue(qreal v);
    //自适应窗口
    void autoFit();
    //鼠标移动事件
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

    //返回当前图片img
    QImage image() const;

public slots:
    //适应窗口大小
    void fitWindow();
    //适应图片大小
    void fitImage();
    //窗口大小改变事件
    void resizeEvent(QResizeEvent *event) override;
    //鼠标滚轮事件
    void wheelEvent(QWheelEvent *event) override;

signals:
    void scaled(qreal perc);

private:
    QGraphicsPixmapItem *m_pixmapItem = nullptr; //当前图像的item
    qreal m_scal = 1.0;
    int resolutionX = 0;
    int resolutionY = 0;
};
