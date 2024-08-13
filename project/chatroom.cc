#include "server/server.h"
#include "server/service.h"
#include "model/historyCasheManager.h"

#include <cstdlib>
#include <signal.h>

std::shared_ptr<CacheManager> cacheManager = std::make_shared<CacheManager>();

void resetHandler(int sig){
    Service::getInstance()->reset();
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress listenAddr(8888);
    Server server(&loop, listenAddr);

    server.start();

    EventLoop* subLoop = server.getNextLoop();

    // 设置定时器，每隔 5 秒刷新一次缓存到数据库
    subLoop->runEvery(1.0, []() {
        cacheManager->flushCacheToDatabase();  // 刷新缓存到数据库
    });

    /*
    loop.runEvery(60.0, []() {
        std::cout << "心跳检测检查" << std::endl;
        Service::getInstance()->checkIfConnAlive();
    });
    */

    loop.loop();

    return 0;
}
