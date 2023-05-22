// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>

class FaceAlign
{
public:
    FaceAlign() = default;
    void setImage(const QImage &image);
    void analyze(float *currentLandmark);
    QImage result() const;

private:
    QImage imageCache;
    QImage lastResult;
};
