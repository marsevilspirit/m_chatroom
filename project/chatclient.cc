#include "client/client.h"
#include "m_netlib/Net/EventLoopThread.h"
#include "client/clientService.h"

#include <iostream>
#include <csignal>
#include <unistd.h>
#include <termios.h>

// 信号处理函数
void signalHandler(int signum) {
    std::cout << "\nctrl+C is disabled\n";
}

// 禁用控制台输入中的EOF（ctrl+D）
void disableCtrlD() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);

    tty.c_cc[VEOF] = _POSIX_VDISABLE;  // 禁用EOF字符

    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

int main(int argc, char *argv[]){
    // 设置信号处理函数
    signal(SIGINT, signalHandler);

    // 禁用ctrl+D
    disableCtrlD();

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    EventLoopThread loopThread;
    InetAddress serverAddr(argv[1], static_cast<uint16_t>(atoi(argv[2])));

    Client client(loopThread.startLoop(), serverAddr);
    client.connect();

    EnterChatRoom(client);

    return 0;
}
