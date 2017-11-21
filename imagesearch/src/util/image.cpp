#include <util/image.h>
#include <vector>

namespace imageutil {
    void MatToString(const cv::Mat& mat, std::string& s) {
        //cv::Size size = mat.size();
        //int total = size.width * size.height * mat.channels();
        //std::vector<unsigned char> data(mat.ptr(), mat.ptr() + total);
        std::vector<unsigned char> data;
        cv::imencode(".jpg", mat, data);
        s = std::string(data.begin(), data.end()); 
    }

    void StringToMat(const std::string& s, cv::Mat& mat) {
        std::vector<unsigned char> data(s.begin(), s.end());
        mat = cv::Mat(cv::imdecode(data, 1));
    }
}
