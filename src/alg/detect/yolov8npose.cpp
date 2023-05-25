// SPDX-FileCopyrightText: 2023 wzyforgit
//
// SPDX-License-Identifier: GPL-3.0-or-later

//此部分代码改编自NCNN社区的triple-Mu的yolov8部署代码的姿态估计部分
//原代码地址为：https://github.com/triple-Mu/ncnn-examples/blob/main/python/yolov8/inference.py
//本文件中的python对比注释仅保留了姿态估计的代码，其它的if-else分支已直接删除

#include "yolov8npose.h"
#include <QtDebug>
#include <cstring>
#include <ncnn/net.h>

Yolov8nPose::~Yolov8nPose()
{
    delete net;
}

void Yolov8nPose::init()
{
    net = new ncnn::Net;
    net->opt.use_vulkan_compute = true;
    net->load_param("/home/fuko/桌面/AI/Detect/sanmu_yolo/python/yolov8/ncnn-models/yolov8n-pose-opt.param");
    net->load_model("/home/fuko/桌面/AI/Detect/sanmu_yolo/python/yolov8/ncnn-models/yolov8n-pose-opt.bin");
}

void Yolov8nPose::setImage(const QImage &image)
{
    imageCache = image.convertToFormat(QImage::Format_RGB888);
}

/*def sigmoid(x: ndarray) -> ndarray:
      return 1. / (1. + np.exp(-x))*/
float sigmoid(float x)
{
    return 1.0f / (1.0f + (std::exp(-x)));
}

float findMax(const QVector<float> &datas)
{
    float maxValue = datas[0];
    for(int i = 1;i != datas.size();++i)
    {
        if(maxValue < datas[i])
        {
            maxValue = datas[i];
        }
    }
    return maxValue;
}

float sumVec(const QVector<float> &datas)
{
    float sum = 0;
    for(auto &data : datas)
    {
        sum += data;
    }
    return sum;
}

/*def softmax(x: ndarray, axis: int = -1) -> ndarray:
      e_x = np.exp(x - np.max(x, axis=axis, keepdims=True))
      y = e_x / e_x.sum(axis=axis, keepdims=True)
      return y*/

QVector<QVector<float>> softmax(const QVector<QVector<float>> &d)
{
    QVector<QVector<float>> result;
    for(auto &datas : d)
    {
        float maxValue = findMax(datas);
        QVector<float> e_x;
        for(auto &data : datas)
        {
            e_x.push_back(std::exp(data - maxValue));
        }

        float e_x_sum = sumVec(e_x);
        for(auto &data : e_x)
        {
            data /= e_x_sum;
        }

        result.push_back(e_x);
    }

    return result;
}

QVector<float> matMul(const QVector<QVector<float>> &v, const QVector<float> &dfl)
{
    QVector<float> result;

    for(auto &v_data : v)
    {
        float currentSum = 0;
        for(int i = 0;i != v_data.size();++i)
        {
            currentSum += v_data[i] * dfl[i];
        }
        result.push_back(currentSum);
    }

    return result;
}

QVector<float> dflDecode(const QVector<float> &dfl, const QVector<float> &boxBuffer, int reg_max)
{
    //1.划分为4 * 16组数据
    QVector<QVector<float>> buffer(boxBuffer.size() / reg_max);
    for(int i = 0;i != buffer.size();++i)
    {
        QVector<float> currentBuffer(boxBuffer.begin() + i * 16, boxBuffer.begin() + (i + 1) * 16);
        buffer[i] = currentBuffer;
    }

    //2.每组数据计算softmax值
    auto softmaxResult = softmax(buffer);

    //3.softmax值与dfl进行矩阵乘法
    return matMul(softmaxResult, dfl);
}

QList<Yolov8nPose::KpsData> kpsDecode(const QVector<float> &kpsBuffer, int stride, int w, int h, float kps_thres)
{
    QList<Yolov8nPose::KpsData> result;
    for(int i = 0;i != kpsBuffer.size();i += 3)
    {
        float score = sigmoid(kpsBuffer[i + 2]);
        Yolov8nPose::KpsData currentResult;
        if(score < kps_thres)
        {
            currentResult.score = -1.0f;
            result.push_back(currentResult); //装一个空数据进去，防止后续解码骨架位置的时候解码失败
            continue;
        }
        currentResult.point = QPoint((kpsBuffer[i] * 2 + w) * stride, (kpsBuffer[i + 1] * 2 + h) * stride);
        currentResult.score = score;
        result.push_back(currentResult);
    }
    return result;
}

QList<Yolov8nPose::DetectResult> postprocess(const QList<ncnn::Mat> &feats, float conf_thres, float kps_thres)
{
    /*dfl = np.arange(0, reg_max, dtype=np.float32)
      scores_pro = []
      boxes_pro = []
      labels_pro = []
      kpss_pro = []*/

    const int reg_max = 16;
    QVector<float> dfl;
    for(int i = 0;i != reg_max;++i)
    {
        dfl.push_back(i);
    }

    /*for i, feat in enumerate(feats):
        stride = 8 << i
        score_feat, box_feat, kps_feat = np.split(feat, [1, 1 + 64], -1)

        score_feat = sigmoid(score_feat)
        _max = score_feat.squeeze(-1)

        indices = np.where(_max > conf_thres)
        hIdx, wIdx = indices
        num_proposal = hIdx.size
        if not num_proposal:
            continue

        scores = _max[hIdx, wIdx]
        boxes = box_feat[hIdx, wIdx].reshape(-1, 4, reg_max)
        boxes = softmax(boxes, -1) @ dfl
        kpss = kps_feat[hIdx, wIdx]

        for k in range(num_proposal):
            h, w = hIdx[k], wIdx[k]
            score = scores[k]
            x0, y0, x1, y1 = boxes[k]

            x0 = (w + 0.5 - x0) * stride
            y0 = (h + 0.5 - y0) * stride
            x1 = (w + 0.5 + x1) * stride
            y1 = (h + 0.5 + y1) * stride

            kps = kpss[k].reshape(-1, 3)
            kps[:, :1] = (kps[:, :1] * 2. + w) * stride
            kps[:, 1:2] = (kps[:, 1:2] * 2. + h) * stride
            kps[:, 2:3] = sigmoid(kps[:, 2:3])

            scores_pro.append(float(score))
            boxes_pro.append(np.array([x0, y0, x1 - x0, y1 - y0], dtype=np.float32))
            kpss_pro.append(kps)*/

    QList<Yolov8nPose::DetectResult> results;
    for(int i = 0;i != feats.size();++i)
    {
        int stride = 8 << i;
        auto feat = feats[i];

        //此处数据为每116一组，包含概率[1]，框的数据[64]，关键点的坐标及分数[(x,y,score)*17=51]
        //因此直接遍历ncnn::Mat进行解码即可
        //注意：ncnn::Mat的c对应py里面的hIdx，h对应py里面的wIdx
        for(int c = 0;c != feat.c;++c)
        {
            const float *ptr = feat.channel(c);
            for(int y = 0;y != feat.h;++y, ptr += feat.w)
            {
                float score = sigmoid(ptr[0]);
                if(score < conf_thres) //分数不达标直接pass
                {
                    continue;
                }

                //box数据解码
                QVector<float> boxBuffer(64);
                std::memcpy(boxBuffer.data(), ptr + 1, sizeof(float) * 64);
                auto boxData = dflDecode(dfl, boxBuffer, reg_max);

                int h = c;
                int w = y;
                float x0 = (w + 0.5 - boxData[0]) * stride;
                float y0 = (h + 0.5 - boxData[1]) * stride;
                float x1 = (w + 0.5 + boxData[2]) * stride;
                float y1 = (h + 0.5 + boxData[3]) * stride;

                //kps数据解码
                QVector<float> kpsBuffer(51);
                std::memcpy(kpsBuffer.data(), ptr + 65, sizeof(float) * 51);
                auto kps = kpsDecode(kpsBuffer, stride, w, h, kps_thres);

                //装载数据
                Yolov8nPose::DetectResult currentResult;
                currentResult.score = score;
                currentResult.bbox = QRectF(x0, y0, x1 - x0, y1 - y0);
                currentResult.kps = kps;
                results.append(currentResult);
            }
        }
    }
    return results;
}

float iou(const QRectF &lhs, const QRectF &rhs)
{
    //1.计算交集区域
    QRectF rectIntersection = lhs & rhs; //交集
    if(rectIntersection.isEmpty()) {
        return 0;
    }
    float intersectionArea = rectIntersection.width() * rectIntersection.height();

    //2.计算并集面积
    float lhsArea = lhs.width() * lhs.height();
    float rhsArea = rhs.width() * rhs.height();
    float unionArea = lhsArea + rhsArea - intersectionArea;

    return intersectionArea / unionArea;
}

QList<Yolov8nPose::DetectResult> nms(const QList<Yolov8nPose::DetectResult> &originResults, float iouThreshold)
{
    QList<Yolov8nPose::DetectResult> result(originResults);

    //概率由大到小排序
    std::sort(result.begin(), result.end(), [](const Yolov8nPose::DetectResult &lhs, const Yolov8nPose::DetectResult &rhs) {
        return lhs.score > rhs.score;
    });

    //去除重合度高且概率低的部分
    for(int i = 0;i != result.size();++i) {
        for(int j = i + 1;j != result.size();++j) {
            if(iou(result[i].bbox, result[j].bbox) > iouThreshold) {
                result.removeAt(j);
                --j;
            }
        }
    }

    return result;
}

void Yolov8nPose::analyze()
{
    if(net == nullptr)
    {
        init();
    }

    /*ex = net.create_extractor()
      img = cv2.imread(file)
      img_w = img.shape[1]
      img_h = img.shape[0]*/

    auto ex = net->create_extractor();
    int img_w = imageCache.width();
    int img_h = imageCache.height();

    /*w = img_w
    h = img_h
    scale = 1.0
    if w > h:
    scale = float(args.input_size) / w
    w = args.input_size
    h = int(h * scale)
    else:
    scale = float(args.input_size) / h
    h = args.input_size
    w = int(w * scale)*/

    int w = img_w;
    int h = img_h;
    int input_size = 640;
    float scale = 1.0f;
    if(w > h)
    {
        scale = float(input_size) / w;
        w = input_size;
        h = int(h * scale);
    }
    else
    {
        scale = float(input_size) / h;
        h = input_size;
        w = int(w * scale);
    }

    /*mat_in = ncnn.Mat.from_pixels_resize(
        img, ncnn.Mat.PixelType.PIXEL_BGR2RGB, img_w, img_h, w, h
        )

    wpad = (w + 31) // 32 * 32 - w
    hpad = (h + 31) // 32 * 32 - h
    mat_in_pad = ncnn.copy_make_border(
        mat_in,
        hpad // 2,
        hpad - hpad // 2,
        wpad // 2,
        wpad - wpad // 2,
        ncnn.BorderType.BORDER_CONSTANT,
        114.0,
        )

    dw = wpad // 2
    dh = hpad // 2

    mat_in_pad.substract_mean_normalize([], [1 / 255.0, 1 / 255.0, 1 / 255.0])*/

    ncnn::Mat mat_in = ncnn::Mat::from_pixels_resize(
                imageCache.bits(), ncnn::Mat::PixelType::PIXEL_RGB, img_w, img_h,imageCache.bytesPerLine(), w, h
                );

    int wpad = (w + 31) / 32 * 32 - w;
    int hpad = (h + 31) / 32 * 32 - h;
    ncnn::Mat mat_in_pad;
    ncnn::copy_make_border(
                mat_in,
                mat_in_pad,
                hpad / 2,
                hpad - hpad / 2,
                wpad / 2,
                wpad - wpad / 2,
                ncnn::BorderType::BORDER_CONSTANT,
                114.0);

    int dw = wpad / 2;
    int dh = hpad / 2;

    static const float norm[] = {1 / 255.0, 1 / 255.0, 1 / 255.0};
    mat_in_pad.substract_mean_normalize(nullptr, norm);

    /*ex.input('images', mat_in_pad)
    ret1, mat_out1 = ex.extract(output_names[0])  # stride 8
    assert not ret1, f'extract {output_names[0]} with something wrong!'
    ret2, mat_out2 = ex.extract(output_names[1])  # stride 16
    assert not ret2, f'extract {output_names[1]} with something wrong!'
    ret3, mat_out3 = ex.extract(output_names[2])  # stride 32
    assert not ret3, f'extract {output_names[2]} with something wrong!'*/
    ex.input("images", mat_in_pad);
    ncnn::Mat mat_out1;
    ncnn::Mat mat_out2;
    ncnn::Mat mat_out3;
    ex.extract("356", mat_out1); //stride 8
    ex.extract("381", mat_out2); //stride 16
    ex.extract("406", mat_out3); //stride 32

    /*outputs = [np.array(mat_out1), np.array(mat_out2), np.array(mat_out3)]
      results = postprocess(outputs, task_type, args.score_thr, 16)*/
    auto results = postprocess({mat_out1, mat_out2, mat_out3}, 0.25, 0.5);

    //indices = cv2.dnn.NMSBoxes(boxes_pro, scores_pro, args.score_thr, args.iou_thr).flatten()
    results = nms(results, 0.5);

    /*for idx in indices:
          box = boxes_pro[idx]
          score = scores_pro[idx]
          clsid = labels_pro[idx]

          color = CLASS_COLORS[clsid]
          box[2:] = box[:2] + box[2:]
          x0, y0, x1, y1 = box

          # clip feature
          x0 = min(max(x0, 1), w - 1)
          y0 = min(max(y0, 1), h - 1)
          x1 = min(max(x1, 1), w - 1)
          y1 = min(max(y1, 1), h - 1)

          x0 = (x0 - dw) / scale
          y0 = (y0 - dh) / scale
          x1 = (x1 - dw) / scale
          y1 = (y1 - dh) / scale

          # clip image
          x0 = min(max(x0, 1), img_w - 1)
          y0 = min(max(y0, 1), img_h - 1)
          x1 = min(max(x1, 1), img_w - 1)
          y1 = min(max(y1, 1), img_h - 1)

          x0, y0, x1, y1 = math.floor(x0), math.floor(y0), math.ceil(x1), math.ceil(y1)

          kps = kpss_pro[idx]
          for i in range(19):
              if i < 17:
                  px, py, ps = kps[i]
                  px = round((px - dw) / scale)
                  py = round((py - dh) / scale)
                  if ps > 0.5:
                      kcolor = KPS_COLORS[i]
                      cv2.circle(img, (px, py), 5, kcolor, -1)
              xi, yi = SKELETON[i]
              pos1_s = kps[xi - 1][2]
              pos2_s = kps[yi - 1][2]
              if pos1_s > 0.5 and pos2_s > 0.5:
                  limb_color = LIMB_COLORS[i]
                  pos1_x = round((kps[xi - 1][0] - dw) / scale)
                  pos1_y = round((kps[xi - 1][1] - dw) / scale)
                  pos2_x = round((kps[yi - 1][0] - dw) / scale)
                  pos2_y = round((kps[yi - 1][1] - dw) / scale)
                  cv2.line(img, (pos1_x, pos1_y), (pos2_x, pos2_y), limb_color, 2)

          cv2.rectangle(img, (x0, y0), (x1, y1), color, 2)
          cv2.putText(img, f'{CLASS_NAMES[clsid]}: {score:.2f}', (x0, y0 - 5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, color,
                      2)*/
    for(auto &eachResult : results)
    {
        //还原bbox
        QRectF bbox = eachResult.bbox;
        float x0 = bbox.x();
        float y0 = bbox.y();
        float x1 = bbox.x() + bbox.width();
        float y1 = bbox.y() + bbox.height();

        x0 = std::min(std::max(x0, 1.0f), w - 1.0f);
        y0 = std::min(std::max(y0, 1.0f), h - 1.0f);
        x1 = std::min(std::max(x1, 1.0f), w - 1.0f);
        y1 = std::min(std::max(y1, 1.0f), h - 1.0f);

        x0 = (x0 - dw) / scale;
        y0 = (y0 - dh) / scale;
        x1 = (x1 - dw) / scale;
        y1 = (y1 - dh) / scale;

        x0 = std::min(std::max(x0, 1.0f), img_w - 1.0f);
        y0 = std::min(std::max(y0, 1.0f), img_h - 1.0f);
        x1 = std::min(std::max(x1, 1.0f), img_w - 1.0f);
        y1 = std::min(std::max(y1, 1.0f), img_h - 1.0f);

        x0 = std::floor(x0);
        y0 = std::floor(y0);
        x1 = std::ceil(x1);
        y1 = std::ceil(y1);

        eachResult.bbox = QRectF(x0, y0, x1 - x0, y1 - y0);

        //还原kps
        for(auto &eachKp : eachResult.kps)
        {
            eachKp.point.setX(std::round((eachKp.point.x() - dw) / scale));
            eachKp.point.setY(std::round((eachKp.point.y() - dh) / scale));
        }
    }
    lastResult = std::move(results);
}

QList<Yolov8nPose::DetectResult> Yolov8nPose::result() const
{
    return lastResult;
}

const QList<QPair<int, int>>& Yolov8nPose::skeleton()
{
    static const QList<QPair<int, int>> data = {{16, 14}, {14, 12}, {17, 15}, {15, 13},
                                                {12, 13}, {6, 12}, {7, 13}, {6, 7},
                                                {6, 8}, {7, 9}, {8, 10}, {9, 11},
                                                {2, 3}, {1, 2}, {1, 3}, {2, 4},
                                                {3, 5}, {4, 6}, {5, 7}};
    return data;
}

const QList<QColor>& Yolov8nPose::kpsColors()
{
    static const QList<QColor> data = {{0, 255, 0}, {0, 255, 0}, {0, 255, 0},
                                             {0, 255, 0}, {0, 255, 0}, {255, 128, 0},
                                             {255, 128, 0}, {255, 128, 0}, {255, 128, 0},
                                             {255, 128, 0}, {255, 128, 0}, {51, 153, 255},
                                             {51, 153, 255}, {51, 153, 255}, {51, 153, 255},
                                             {51, 153, 255}, {51, 153, 255}};
    return data;
}

const QList<QColor>& Yolov8nPose::limbColors()
{
    static const QList<QColor> data = {{51, 153, 255}, {51, 153, 255}, {51, 153, 255},
                                              {51, 153, 255}, {255, 51, 255}, {255, 51, 255},
                                              {255, 51, 255}, {255, 128, 0}, {255, 128, 0},
                                              {255, 128, 0}, {255, 128, 0}, {255, 128, 0},
                                              {0, 255, 0}, {0, 255, 0}, {0, 255, 0},
                                              {0, 255, 0}, {0, 255, 0}, {0, 255, 0},
                                              {0, 255, 0}};
    return data;
}
