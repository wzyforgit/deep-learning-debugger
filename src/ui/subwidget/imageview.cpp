// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

//改编自：https://github.com/linuxdeepin/deepin-ocr/blob/master/src/view/imageview.cpp
//原版权所有者：UnionTech Software Technology Co., Ltd.

#include "imageview.h"

#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QtMath>

ImageView::ImageView(QWidget *parent)
    : QGraphicsView(parent)
    , m_pixmapItem(new QGraphicsPixmapItem)
{
    setMouseTracking(true);
    setDragMode(ScrollHandDrag);
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->setCursor(Qt::ArrowCursor);

    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    this->scene()->addItem(m_pixmapItem);
}

void ImageView::setImage(const QImage &image)
{
    m_pixmapItem->setPixmap(QPixmap::fromImage(image));
    setSceneRect(m_pixmapItem->boundingRect());

    //视频播放用
    if(image.width() != resolutionX || image.height() != resolutionY) {
        autoFit();
        resolutionX = image.width();
        resolutionY = image.height();
    }
}

qreal ImageView::windowRelativeScale() const
{
    QRectF bf = sceneRect();
    if (1.0 * width() / height() > 1.0 * bf.width() / bf.height()) {
        return 1.0 * height() / bf.height();
    } else {
        return 1.0 * width() / bf.width();
    }
}

void ImageView::fitWindow()
{
    qreal wrs = windowRelativeScale() * 0.95;
    m_scal = wrs;
    resetTransform();
    scale(wrs, wrs);
}

void ImageView::fitImage()
{
    resetTransform();
    m_scal = 1.0;
    scale(1, 1);
}

qreal ImageView::imageRelativeScale() const
{
    return transform().m11() / devicePixelRatioF();
}

void ImageView::autoFit()
{
    if (image().isNull())
        return;

    QSize image_size = image().size();
    if ((image_size.width() >= width() || image_size.height() >= height() - 150) &&
            width() > 0 &&
            height() > 0) {
        fitWindow();
    } else {
        fitImage();
    }
}

void ImageView::mouseReleaseEvent(QMouseEvent *e)
{
    QGraphicsView::mouseReleaseEvent(e);
    viewport()->setCursor(Qt::ArrowCursor);
}

void ImageView::mousePressEvent(QMouseEvent *e)
{
    QGraphicsView::mousePressEvent(e);
    viewport()->unsetCursor();
    viewport()->setCursor(Qt::ArrowCursor);
}

void ImageView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() | Qt::NoButton)) {
        viewport()->setCursor(Qt::ArrowCursor);
    } else {
        QGraphicsView::mouseMoveEvent(event);
        viewport()->setCursor(Qt::ClosedHandCursor);
    }
}

QImage ImageView::image() const
{
    if (m_pixmapItem) {
        return m_pixmapItem->pixmap().toImage();
    } else {
        return QImage();
    }
}

void ImageView::resizeEvent(QResizeEvent *event)
{
    autoFit();
    return QGraphicsView::resizeEvent(event);
}

void ImageView::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(1.2, event->angleDelta().y() / 240.0);
    scaleAtPoint(event->position(), factor);

    event->accept();
}

void ImageView::scaleAtPoint(QPointF pos, qreal factor)
{
    const QPointF targetScenePos = mapToScene(pos.toPoint());
    setScaleValue(factor);
    const QPointF curPos = mapFromScene(targetScenePos);
    const QPointF centerPos = QPointF(width() / 2.0, height() / 2.0) + (curPos - pos);
    const QPointF centerScenePos = mapToScene(centerPos.toPoint());
    centerOn(static_cast<int>(centerScenePos.x()), static_cast<int>(centerScenePos.y()));
}

void ImageView::setScaleValue(qreal v)
{
    m_scal *= v;
    scale(v, v);
    if (v < 1 && m_scal < 0.03) {
        const qreal minv = 0.029 / m_scal;
        scale(minv, minv);
        m_scal *= minv;
    } else if (v > 1 && m_scal > 20) {
        const qreal maxv = 20.0 / m_scal;
        scale(maxv, maxv);
        m_scal *= maxv;
    }

    emit scaled(m_scal * 100);
}
