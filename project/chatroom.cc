#include "server/server.h"
#include "server/service.h"

#include <signal.h>

void resetHandler(int sig){
    Service::getInstance()->reset();
}

int main()
{
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress listenAddr(8888);
    Server server(&loop, listenAddr);

    server.start();

    loop.loop();

    return 0;
}
