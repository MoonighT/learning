#pragma once

#include <opencv2/opencv.hpp>
#include <string>

namespace imageutil {
    // convert from std string and cv mat
    void MatToString(const cv::Mat&, std::string&); 
    void StringToMat(const std::string&, cv::Mat&);
}
