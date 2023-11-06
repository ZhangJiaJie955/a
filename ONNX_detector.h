#if !defined(ONNX_RUNTIME_DETECTOR_H)
#define ONNX_RUNTIME_DETECTOR_H


# pragma once


#include <iostream>
#include <vector>
#include <numeric>
#include <string>
#include <functional>

#include <onnxruntime_c_api.h>
#include <onnxruntime_cxx_api.h>
#include <cuda_provider_factory.h>
#include "utils_anpr_detect.h"
class OnnxDetector {
public:
    OnnxDetector(Ort::Env& env, const void* model_data, size_t model_data_length, const Ort::SessionOptions& options);
    OnnxDetector(Ort::Env& env, const ORTCHAR_T* model_path, const Ort::SessionOptions& options);


    void dump() const;

    std::vector<std::vector<Detection>>
    Run(const cv::Mat& img, float conf_threshold, float iou_threshold, bool preserve_aspect_ratio);

    std::vector<std::tuple<cv::Rect, float, int>>
        Run_v1(const cv::Mat& img, float conf_threshold, float iou_threshold);
private:

protected :
    Ort::SessionOptions sessionOptions;
    Ort::Session session;
    const Ort::Env& env;
#ifdef LPR_EDITOR_USE_CUDA
    bool useCUDA;
#endif //LPR_EDITOR_USE_CUDA
};



void nms(const std::vector<cv::Rect>& srcRects, std::vector<cv::Rect>& resRects, std::vector<int>& resIndexs, float thresh);

template <typename T>
T vectorProduct(const std::vector<T>& v)
{
    return std::accumulate(v.begin(), v.end(), 1, std::multiplies<T>());
}

#endif // !defined(ONNX_RUNTIME_DETECTOR_H)