#pragma once

#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/features2d.hpp>

namespace imagefeature {
    class FeatureExtractor {
        public:
        FeatureExtractor();
        ~FeatureExtractor() {};
        void GetImageFeature(const std::string, cv::Mat&);
        void GetImageFeature(const std::vector<char>, cv::Mat&);
        void GetImagesSimilarity(const cv::Mat&, const cv::Mat&, double& /*out*/);

        private:
        cv::FeatureDetector* detector;
        cv::DescriptorExtractor* extractor;
        cv::FlannBasedMatcher* matcher;
    };
}
