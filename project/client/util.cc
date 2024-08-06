#include "util.h"

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
