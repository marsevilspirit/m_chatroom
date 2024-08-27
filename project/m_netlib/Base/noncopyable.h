// Created by mars on 2/8/24.

#ifndef MARS_BASE_NONCOPYABLE_H
#define MARS_BASE_NONCOPYABLE_H

namespace mars {

struct base_token {};

class noncopyable : base_token {
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

protected: //防止直接实例化
    noncopyable() = default;
    ~noncopyable() = default;
};

} // namespace mars

#endif // MARS_BASE_NONCOPYABLE_H
