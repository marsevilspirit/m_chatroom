#include "server/server.h"
#include "server/service.h"
#include "model/historyCasheManager.h"

#include <signal.h>

std::shared_ptr<CacheManager> cacheManager = std::make_shared<CacheManager>();

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

    // 设置定时器，每隔 5 秒刷新一次缓存到数据库
    loop.runEvery(5.0, []() {
        cacheManager->flushCacheToDatabase();  // 刷新缓存到数据库
    });

    loop.loop();

    return 0;
}
