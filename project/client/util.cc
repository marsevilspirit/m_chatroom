#include "util.h"
#include <limits>
#include <termios.h>
#include <unistd.h>
#include <algorithm>

// 将字节数组转换为十六进制字符串
std::string bytesToHex(const unsigned char* bytes, size_t length)
{
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
    }
    return ss.str();
}

// 使用EVP接口将密码转换为SHA-256哈希值
std::string passwordToSha256(const std::string &password)
{
    EVP_MD_CTX* mdctx;
    const EVP_MD* md;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    // 创建和初始化EVP_MD_CTX上下文
    mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        std::cerr << "Error creating EVP_MD_CTX" << std::endl;
        return "";
    }

    // 选择SHA-256算法
    md = EVP_sha256();
    if (!EVP_DigestInit_ex(mdctx, md, NULL)) {
        std::cerr << "Error initializing digest" << std::endl;
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    // 更新哈希上下文
    if (!EVP_DigestUpdate(mdctx, password.c_str(), password.size())) {
        std::cerr << "Error updating digest" << std::endl;
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    // 完成哈希计算
    if (!EVP_DigestFinal_ex(mdctx, hash, &hash_len)) {
        std::cerr << "Error finalizing digest" << std::endl;
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    // 释放上下文
    EVP_MD_CTX_free(mdctx);

    return bytesToHex(hash, hash_len);
}

void clearInputBuffer() 
{
    std::cin.clear(); // 清除输入状态标志
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 忽略剩余输入
}

std::string getPassword() {
    std::string password;
    char ch;
    struct termios oldt, newt;

    // 获取终端的当前属性
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // 关闭回显功能
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // 输入密码
    std::getline(std::cin, password);

    // 恢复终端的原始属性
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;

    return password;
}

bool isNumber(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}
