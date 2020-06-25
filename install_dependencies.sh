#!/usr/bin/env bash

set -e 


install_dir="$(pwd)/external"

cd /tmp
rm -rf HdrHistogram_c
echo "Install dependencies: HdrHistogram_c to ${install_dir} ..."
git clone https://github.com/HdrHistogram/HdrHistogram_c.git
cd HdrHistogram_c
mkdir build && cd build 
cmake .. -DCMAKE_INSTALL_PREFIX=${install_dir}
make && make install 
cd /tmp 
rm -rf HdrHistogram_c

rm -rf googletest
echo "Install dependencies: googletest to ${install_dir} ..."
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build 
cmake .. -DCMAKE_INSTALL_PREFIX=${install_dir}
make && make install 
cd /tmp 
rm -rf googletest

rm -rf fmt 
echo "Install dependencies: fmtlib to ${install_dir} ..."
git clone https://github.com/xlong88/fmt.git
cd fmt 
mkdir build && cd build 
cmake .. -DCMAKE_INSTALL_PREFIX=${install_dir} -DFMT_TEST=False
make && make install 
cd /tmp 
rm -rf fmt

sudo apt-get install python3-pip
pip3 install --user -r py/requirements.txt

