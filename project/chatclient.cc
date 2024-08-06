#include "client/client.h"
#include "m_netlib/Net/EventLoop.h"

int main(int argc, char *argv[]){
    EventLoop loop;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    InetAddress serverAddr(argv[1], static_cast<uint16_t>(atoi(argv[2])));
    Client client(&loop, serverAddr);
    client.connect();
    loop.loop();

    return 0;
}
