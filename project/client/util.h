//Created by mars on 7/8/24

#ifndef MARS_UTIL_H
#define MARS_UTIL_H

#include <string>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <iostream>
#include <iomanip>
#include <sstream>

//用哈希函数给密码加密
std::string passwordToSha256(const std::string &password);

void clearInputBuffer();

std::string getPassword();

bool isNumber(const std::string& s);

#endif //MARS_UTIL_H
