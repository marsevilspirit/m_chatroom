#include "server/server.h"
#include "server/service.h"
#include "model/historyCasheManager.h"
#include "threadpool/threadpool.h"

#include <cstdlib>
#include <signal.h>

std::shared_ptr<CacheManager> cacheManager = std::make_shared<CacheManager>();
std::shared_ptr<threadpool> mysqlInsertThreadPool = std::make_shared<threadpool>(10);

void resetHandler(int sig){
    Service::getInstance()->reset();
    exit(EXIT_SUCCESS);
}

int main()
{
    LogDebug("111111111111111111111");
    
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress listenAddr(8888);
    Server server(&loop, listenAddr);

    server.start();

    EventLoop* subLoop1 = server.getNextLoop();

    // 设置定时器，每隔 5 秒刷新一次缓存到数据库
    subLoop1->runEvery(1.0, []() {
            mysqlInsertThreadPool->enqueue([]() {
                cacheManager->flushCacheToDatabase();
            });
    });

    EventLoop* subLoop2 = server.getNextLoop();

    subLoop2->runEvery(300.0, []() {
        LogInfo("心跳检测检查")
        Service::getInstance()->checkIfConnAlive();
    });

    loop.loop();

    return 0;
}
