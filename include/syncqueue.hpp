// syncqueue.hpp
#pragma once
#include <queue>
#include "syncflag.hpp"

template <typename T>
class SyncQueue {
public:
    void Push(const T& item) {
        SyncWriteHold lock(&flag);
        queue.push(item);
    }

    bool Pop(T& out) {
        SyncWriteHold lock(&flag);
        if (queue.empty()) return false;
        out = queue.front();
        queue.pop();
        return true;
    }

    size_t Size() const {
        SyncReadHold lock(&flag);
        return queue.size();
    }

    bool Empty() const {
        SyncReadHold lock(&flag);
        return queue.empty();
    }

private:
    std::queue<T> queue;
    mutable SyncFlag flag;
};
;