#include <imagefeature/feature_extractor.h>
#include <vector>

namespace imagefeature {
    FeatureExtractor::FeatureExtractor() {
        // init once    
        cv::initModule_nonfree();//THIS LINE IS IMPORTANT   
        detector = new cv::SiftFeatureDetector(
                0, // nFeatures
                4, // nOctaveLayers
                0.04, // contrastThreshold
                10, //edgeThreshold
                1.6 //sigma
                );
        extractor = new cv::SiftDescriptorExtractor();
        matcher = new cv::FlannBasedMatcher();
    }

    void FeatureExtractor::GetImageFeature(const std::vector<char> imagedata, cv::Mat& descriptors/*out*/) {
        cv::Mat image = cv::imdecode(imagedata, cv::IMREAD_ANYCOLOR);
        std::vector<cv::KeyPoint> keypoints;
        detector->detect(image, keypoints);
        extractor->compute(image, keypoints, descriptors);
    }

    void FeatureExtractor::GetImageFeature(const std::string imagepath, cv::Mat& descriptors/*out*/) {
        cv::Mat image = cv::imread(imagepath, CV_LOAD_IMAGE_GRAYSCALE);
        std::vector<cv::KeyPoint> keypoints;
        detector->detect(image, keypoints);
        extractor->compute(image, keypoints, descriptors);
    }


    void FeatureExtractor::GetImagesSimilarity(const cv::Mat& desc1, const cv::Mat& desc2, double& score/*out*/) {
        std::vector<cv::DMatch> matches;
        matcher->match(desc1, desc2, matches);
        double max_dist = 0; double min_dist = 100;

        //-- Quick calculation of max and min distances between keypoints
        for(size_t i = 0; i < matches.size(); i++){ 
            double dist = matches[i].distance;
            if(dist < min_dist) min_dist = dist;
            if(dist > max_dist) max_dist = dist;
        }
        int good_matches;
        for(size_t i = 0; i < matches.size(); i++){ 
            if( matches[i].distance <= cv::max(2*min_dist, 0.02)  ){ 
                good_matches++;
            }
        }
        score = good_matches * 1.0 / matches.size();
    }
}
