cmake_minimum_required( VERSION 2.8  )
project( image_search)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -std=c++11")

# Cmake find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Protobuf
# gRPC
find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)

set(PROTOS
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/image_search.proto
    )

# proto src
set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto-src)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})
include_directories(${PROTO_SRC_DIR})

# generate proto
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})

# opencv
set( OpenCV_DIR "/usr/share/OpenCV/"  )
find_package( OpenCV REQUIRED  )

# include
set(IMAGESEARCH_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/include)
include_directories(${IMAGESEARCH_SRC_INCLUDE_DIR})

# source
file(GLOB_RECURSE srcs ${PROJECT_SOURCE_DIR}/src/*/*.cpp)

# Building server
add_executable( image_server
    ${CMAKE_CURRENT_SOURCE_DIR}/src/server/main.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
    ${srcs}
    )
target_link_libraries(image_server
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    glog
    leveldb
    snappy
    ${OpenCV_LIBS}
    )

