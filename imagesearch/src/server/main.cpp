#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <iterator>
#include <vector>

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>

#include <image_search.pb.h>
#include <image_search.grpc.pb.h>

#include <imagefeature/feature_extractor.h>
#include <storage/leveldbstore.h>
#include <util/image.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using namespace shopeeimage;
using namespace imagefeature;
using namespace featurestorage;

class ShopeeImageImpl final : public ShopeeImage::Service {
    public:
        ShopeeImageImpl() {
            f = new FeatureExtractor(); 
            s = new LeveldbStore<std::string, std::string>("test.db");
            // test purpose
            cv::Mat result;
            std::string key = "test.jpg";
            std::ifstream input( key, std::ios::binary);
            std::vector<char> buffer((
                std::istreambuf_iterator<char>(input)), 
                (std::istreambuf_iterator<char>()));
            
            //f->GetImageFeature(key, result);
            f->GetImageFeature(buffer, result);
            std::cout<< result.rows << " - "  << result.cols << std::endl;
            {
                std::string encStr;
                imageutil::MatToString(result, encStr);
                s->Insert(key, encStr);
            }
            {
                std::string encStr; 
                s->Get(key, &encStr);
                imageutil::StringToMat(encStr, result);
            } 
            std::cout<< result.rows << " - "  << result.cols << std::endl;
        }
        Status SearchImage(ServerContext* context, const SearchReq* req,
                SearchResp* resp) override {
            std::cout << "search image request" << std::endl;
            //Iterator* iter = db_->NewIterator(ReadOptions());
            //for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
            //    std::string s = IterStatus(iter);
            //    result.push_back('(');
            //    result.append(s);
            //    result.push_back(')');
            //    forward.push_back(s);
            //}
            //')')
            // calculate score for each pair 
            // store top k highest id
            return Status::OK;
        }

        Status IndexImage(ServerContext* context, const IndexReq* req,
                IndexResp* resp) override {
            std::cout << "index image request" << std::endl;
            cv::Mat result;
            for(int i=0; i<req->images_size(); ++i) {
                std::string queryData = req->images(i).data();
                std::vector<char> buffer(queryData.begin(), queryData.end());
                f->GetImageFeature(buffer, result);
                std::string encStr;
                imageutil::MatToString(result, encStr);
                s->Insert(req->images(i).id(), encStr);
            }
            return Status::OK;
        }
    private:
        // feature extractor
        FeatureExtractor* f;
        // feature storage 
        LeveldbStore<std::string, std::string>* s;
};

void RunServer() {
    std::string server_address("0.0.0.0:8888");
    ShopeeImageImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    LOG(INFO) << "start image server";
    RunServer();
    return 0;
}
