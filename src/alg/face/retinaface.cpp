// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

//此部分代码改编自ncnn下的retinaface demo，主要修改内容为将opencv处理的部分转向使用Qt进行处理，同时适配结果输出接口
//原代码地址为：https://github.com/Tencent/ncnn/blob/master/examples/retinaface.cpp

#include "retinaface.h"
#include <ncnn/net.h>

static float intersection_area(const QRectF &lhs, const QRectF &rhs)
{
    QRectF rectIntersection = lhs & rhs; //交集
    if(rectIntersection.isEmpty()) {
        return 0;
    }
    float intersectionArea = rectIntersection.width() * rectIntersection.height();
    return intersectionArea;
}

static void qsort_descent_inplace(std::vector<RetinaFace::DetectResult>& faceobjects, int left, int right)
{
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j)
    {
        while (faceobjects[i].prob > p)
            i++;

        while (faceobjects[j].prob < p)
            j--;

        if (i <= j)
        {
            // swap
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if (left < j) qsort_descent_inplace(faceobjects, left, j);
        }
        #pragma omp section
        {
            if (i < right) qsort_descent_inplace(faceobjects, i, right);
        }
    }
}

static void qsort_descent_inplace(std::vector<RetinaFace::DetectResult>& faceobjects)
{
    if (faceobjects.empty())
        return;

    qsort_descent_inplace(faceobjects, 0, faceobjects.size() - 1);
}

static void nms_sorted_bboxes(const std::vector<RetinaFace::DetectResult>& faceobjects, std::vector<int>& picked, float nms_threshold)
{
    picked.clear();

    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++)
    {
        areas[i] = faceobjects[i].bbox.width() * faceobjects[i].bbox.height();
    }

    for (int i = 0; i < n; i++)
    {
        const RetinaFace::DetectResult& a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++)
        {
            const RetinaFace::DetectResult& b = faceobjects[picked[j]];

            // intersection over union
            float inter_area = intersection_area(a.bbox, b.bbox);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            //             float IoU = inter_area / union_area
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}

static ncnn::Mat generate_anchors(int base_size, const ncnn::Mat& ratios, const ncnn::Mat& scales)
{
    int num_ratio = ratios.w;
    int num_scale = scales.w;

    ncnn::Mat anchors;
    anchors.create(4, num_ratio * num_scale);

    const float cx = base_size * 0.5f;
    const float cy = base_size * 0.5f;

    for (int i = 0; i < num_ratio; i++)
    {
        float ar = ratios[i];

        int r_w = round(base_size / sqrt(ar));
        int r_h = round(r_w * ar); //round(base_size * sqrt(ar));

        for (int j = 0; j < num_scale; j++)
        {
            float scale = scales[j];

            float rs_w = r_w * scale;
            float rs_h = r_h * scale;

            float* anchor = anchors.row(i * num_scale + j);

            anchor[0] = cx - rs_w * 0.5f;
            anchor[1] = cy - rs_h * 0.5f;
            anchor[2] = cx + rs_w * 0.5f;
            anchor[3] = cy + rs_h * 0.5f;
        }
    }

    return anchors;
}

static void generate_proposals(const ncnn::Mat& anchors, int feat_stride,
                               const ncnn::Mat& score_blob, const ncnn::Mat& bbox_blob, const ncnn::Mat& landmark_blob, float prob_threshold,
                               std::vector<RetinaFace::DetectResult>& faceobjects)
{
    int w = score_blob.w;
    int h = score_blob.h;

    // generate face proposal from bbox deltas and shifted anchors
    const int num_anchors = anchors.h;

    for (int q = 0; q < num_anchors; q++)
    {
        const float* anchor = anchors.row(q);

        const ncnn::Mat score = score_blob.channel(q + num_anchors);
        const ncnn::Mat bbox = bbox_blob.channel_range(q * 4, 4);
        const ncnn::Mat landmark = landmark_blob.channel_range(q * 10, 10);

        // shifted anchor
        float anchor_y = anchor[1];

        float anchor_w = anchor[2] - anchor[0];
        float anchor_h = anchor[3] - anchor[1];

        for (int i = 0; i < h; i++)
        {
            float anchor_x = anchor[0];

            for (int j = 0; j < w; j++)
            {
                int index = i * w + j;

                float prob = score[index];

                if (prob >= prob_threshold)
                {
                    // apply center size
                    float dx = bbox.channel(0)[index];
                    float dy = bbox.channel(1)[index];
                    float dw = bbox.channel(2)[index];
                    float dh = bbox.channel(3)[index];

                    float cx = anchor_x + anchor_w * 0.5f;
                    float cy = anchor_y + anchor_h * 0.5f;

                    float pb_cx = cx + anchor_w * dx;
                    float pb_cy = cy + anchor_h * dy;

                    float pb_w = anchor_w * exp(dw);
                    float pb_h = anchor_h * exp(dh);

                    float x0 = pb_cx - pb_w * 0.5f;
                    float y0 = pb_cy - pb_h * 0.5f;
                    float x1 = pb_cx + pb_w * 0.5f;
                    float y1 = pb_cy + pb_h * 0.5f;

                    RetinaFace::DetectResult obj;
                    obj.bbox.setX(x0);
                    obj.bbox.setY(y0);
                    obj.bbox.setWidth(x1 - x0 + 1);
                    obj.bbox.setHeight(y1 - y0 + 1);
                    obj.landmark[0] = cx + (anchor_w + 1) * landmark.channel(0)[index];
                    obj.landmark[1] = cy + (anchor_h + 1) * landmark.channel(1)[index];
                    obj.landmark[2] = cx + (anchor_w + 1) * landmark.channel(2)[index];
                    obj.landmark[3] = cy + (anchor_h + 1) * landmark.channel(3)[index];
                    obj.landmark[4] = cx + (anchor_w + 1) * landmark.channel(4)[index];
                    obj.landmark[5] = cy + (anchor_h + 1) * landmark.channel(5)[index];
                    obj.landmark[6] = cx + (anchor_w + 1) * landmark.channel(6)[index];
                    obj.landmark[7] = cy + (anchor_h + 1) * landmark.channel(7)[index];
                    obj.landmark[8] = cx + (anchor_w + 1) * landmark.channel(8)[index];
                    obj.landmark[9] = cy + (anchor_h + 1) * landmark.channel(9)[index];
                    obj.prob = prob;

                    faceobjects.push_back(obj);
                }

                anchor_x += feat_stride;
            }

            anchor_y += feat_stride;
        }
    }
}

RetinaFace::RetinaFace()
{
}

RetinaFace::~RetinaFace()
{
    delete net;
}

void RetinaFace::init()
{
    net = new ncnn::Net;
    net->opt.use_vulkan_compute = true;
    net->load_param("/home/fuko/桌面/AI/face/mnet.25-opt.param");
    net->load_model("/home/fuko/桌面/AI/face/mnet.25-opt.bin");
}

void RetinaFace::setImage(const QImage &image)
{
    imageCache = image.convertToFormat(QImage::Format_RGB888);
}

void RetinaFace::analyze()
{
    if(net == nullptr)
    {
        init();
    }

    const float prob_threshold = 0.8f;
    const float nms_threshold = 0.4f;

    ncnn::Mat input = ncnn::Mat::from_pixels(imageCache.bits(), ncnn::Mat::PIXEL_RGB, imageCache.width(), imageCache.height(), imageCache.bytesPerLine());

    ncnn::Extractor extractor = net->create_extractor();
    extractor.input("data", input);

    std::vector<RetinaFace::DetectResult> faceproposals;

    // stride 32
    {
        ncnn::Mat score_blob, bbox_blob, landmark_blob;
        extractor.extract("face_rpn_cls_prob_reshape_stride32", score_blob);
        extractor.extract("face_rpn_bbox_pred_stride32", bbox_blob);
        extractor.extract("face_rpn_landmark_pred_stride32", landmark_blob);

        const int base_size = 16;
        const int feat_stride = 32;
        ncnn::Mat ratios(1);
        ratios[0] = 1.f;
        ncnn::Mat scales(2);
        scales[0] = 32.f;
        scales[1] = 16.f;
        ncnn::Mat anchors = generate_anchors(base_size, ratios, scales);

        std::vector<RetinaFace::DetectResult> faceobjects32;
        generate_proposals(anchors, feat_stride, score_blob, bbox_blob, landmark_blob, prob_threshold, faceobjects32);

        faceproposals.insert(faceproposals.end(), faceobjects32.begin(), faceobjects32.end());
    }

    // stride 16
    {
        ncnn::Mat score_blob, bbox_blob, landmark_blob;
        extractor.extract("face_rpn_cls_prob_reshape_stride16", score_blob);
        extractor.extract("face_rpn_bbox_pred_stride16", bbox_blob);
        extractor.extract("face_rpn_landmark_pred_stride16", landmark_blob);

        const int base_size = 16;
        const int feat_stride = 16;
        ncnn::Mat ratios(1);
        ratios[0] = 1.f;
        ncnn::Mat scales(2);
        scales[0] = 8.f;
        scales[1] = 4.f;
        ncnn::Mat anchors = generate_anchors(base_size, ratios, scales);

        std::vector<RetinaFace::DetectResult> faceobjects16;
        generate_proposals(anchors, feat_stride, score_blob, bbox_blob, landmark_blob, prob_threshold, faceobjects16);

        faceproposals.insert(faceproposals.end(), faceobjects16.begin(), faceobjects16.end());
    }

    // stride 8
    {
        ncnn::Mat score_blob, bbox_blob, landmark_blob;
        extractor.extract("face_rpn_cls_prob_reshape_stride8", score_blob);
        extractor.extract("face_rpn_bbox_pred_stride8", bbox_blob);
        extractor.extract("face_rpn_landmark_pred_stride8", landmark_blob);

        const int base_size = 16;
        const int feat_stride = 8;
        ncnn::Mat ratios(1);
        ratios[0] = 1.f;
        ncnn::Mat scales(2);
        scales[0] = 2.f;
        scales[1] = 1.f;
        ncnn::Mat anchors = generate_anchors(base_size, ratios, scales);

        std::vector<RetinaFace::DetectResult> faceobjects8;
        generate_proposals(anchors, feat_stride, score_blob, bbox_blob, landmark_blob, prob_threshold, faceobjects8);

        faceproposals.insert(faceproposals.end(), faceobjects8.begin(), faceobjects8.end());
    }

    // sort all proposals by score from highest to lowest
    qsort_descent_inplace(faceproposals);

    // apply nms with nms_threshold
    std::vector<int> picked;
    nms_sorted_bboxes(faceproposals, picked, nms_threshold);

    int face_count = picked.size();

    lastResult.clear();
    lastResult.resize(face_count);

    QVector<DetectResult> faceobjects(face_count);
    for (int i = 0; i < face_count; i++)
    {
        faceobjects[i] = faceproposals[picked[i]];

        // clip to image size
        float x0 = faceobjects[i].bbox.x();
        float y0 = faceobjects[i].bbox.y();
        float x1 = x0 + faceobjects[i].bbox.width();
        float y1 = y0 + faceobjects[i].bbox.height();

        x0 = std::max(std::min(x0, (float)imageCache.width()  - 1), 0.f);
        y0 = std::max(std::min(y0, (float)imageCache.height() - 1), 0.f);
        x1 = std::max(std::min(x1, (float)imageCache.width()  - 1), 0.f);
        y1 = std::max(std::min(y1, (float)imageCache.height() - 1), 0.f);

        faceobjects[i].bbox.setX(x0);
        faceobjects[i].bbox.setY(y0);
        faceobjects[i].bbox.setWidth(x1 - x0);
        faceobjects[i].bbox.setHeight(y1 - y0);
    }

    //刷入数据
    lastResult = std::move(faceobjects);
}

QVector<RetinaFace::DetectResult> RetinaFace::result() const
{
    return lastResult;
}
