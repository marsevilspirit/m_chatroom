if [ -d "build" ]; then
    rm -rf build
fi 

mkdir build
cd build
cmake ..
make

mkdir Chatroom
mkdir Chatclient

mv chatroom ./Chatroom
mv chatclient ./Chatclient

cp ../project/m_netlib/Log/logconf.json ./Chatroom/logconf.json
cp ../project/m_netlib/Log/logconf.json ./Chatclient/logconf.json
