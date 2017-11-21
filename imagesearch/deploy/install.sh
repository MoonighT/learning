# source: http://docs.opencv.org/2.4/doc/tutorials/introduction/linux_install/linux_install.html

# install dependencies
echo "install setup ..."

apt-get update
apt-get install -y wget unzip
apt-get install -y build-essential checkinstall cmake git libopencv-dev libgtk2.0-dev pkg-config libavcodec-dev libpng12-dev libavformat-dev libswscale-dev yasm libxine2 libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libv4l-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev x264 v4l-utils

# optional packages
apt-get install -y python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff5-dev libjasper-dev libdc1394-22-dev

# download opencv-2.4.13
wget http://downloads.sourceforge.net/project/opencvlibrary/opencv-unix/2.4.13/opencv-2.4.13.zip
unzip opencv-2.4.13.zip && cd opencv-2.4.13
mkdir release && cd release
# compile and install
cmake -G "Unix Makefiles" -D CMAKE_CXX_COMPILER=/usr/bin/g++ -D CMAKE_C_COMPILER=/usr/bin/gcc -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=ON -D BUILD_NEW_PYTHON_SUPPORT=ON -D WITH_V4L=ON -D INSTALL_C_EXAMPLES=ON -D INSTALL_PYTHON_EXAMPLES=ON -D BUILD_EXAMPLES=ON -DWITH_OPENGL=ON -D BUILD_FAT_JAVA_LIB=ON -D INSTALL_TO_MANGLED_PATHS=ON -D INSTALL_CREATE_DISTRIB=ON -D INSTALL_TESTS=ON -D ENABLE_FAST_MATH=ON -D WITH_IMAGEIO=ON -D BUILD_SHARED_LIBS=OFF -D WITH_GSTREAMER=ON ..
make all -j2
make install 

# install non free package
apt-get install -y software-properties-common python-software-properties
add-apt-repository --yes ppa:xqms/opencv-nonfree
apt-get update 
apt-get install -y libopencv-nonfree-dev

# install grpc
apt-get install -y libtool libgflags-dev libgtest-dev clang libc++-dev

cd /workspace
git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
cd grpc
git submodule update --init
make
make install

cd /workspace
# install protobuf
apt-get install -y autoconf automake libtool curl make g++ unzip
git clone https://github.com/google/protobuf.git
cd protobuf
./autogen.sh
./configure
make
make check
make install
ldconfig # refresh shared library cache.

# leveldb
cd /workspace
apt-get install -y libsnappy-dev
wget https://github.com/google/leveldb/archive/v1.20.tar.gz
tar -xzf v1.20.tar.gz
cd leveldb-1.20
make
scp out-static/lib* out-shared/lib* /usr/local/lib/
cd include
cp -R leveldb /usr/local/include
ldconfig

# tool
cd /workspace
apt-get install -y vim

# glog
# gflag

# memory check
