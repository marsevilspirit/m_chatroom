#!/bin/bash
if [ -d "build" ]; then
    rm -rf build
fi 

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j8

cp ../project/m_netlib/Log/logconf.json logconf.json

mkdir received_files
