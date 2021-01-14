#pragma once

#include <opencv2/imgproc.hpp>

#include <torch/script.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/cuda.hpp>
#include <memory>
#include <vector>
#include <string>

class inference_engine {
    public:
        inference_engine(const std::string & compiled_engine_path);

        // ret[0]: ht map
        // ret[1]: lt map
<<<<<<< HEAD
        std::vector<cv::Mat> infer_one(const cv::Mat & rgb_img, bool ret_uint8_flag);
=======
        std::vector<cv::Mat> infer_one(const cv::Mat & rgb_img);
>>>>>>> origin/master
    
    private:
        torch::jit::script::Module engine;
        std::vector<torch::jit::IValue> input_buffer;
};