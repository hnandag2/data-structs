//
//  ThreadSafeQueue.h
//
//  single lock (ie: not fine-grained) thread-safe queue implemented using a mutex
//  and a condition variable. All accesses will be serialized, however waitAndPop()
//  will always wait for the queue to not be empty.
//
//  Design is based off the tutorial in Anthony Williams' C++ Concurrency In Action
//  book (Chapter 6).- https://www.oreilly.com/library/view/c-concurrency-in/9781933988771/
//
//  Created by Nandagopal, Harihar on 12/24/18.
//

#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>


template <typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() {}
    
    bool tryPop(T& value)
    {
        std::lock_guard<std::mutex> lg(mLock);
        if (mDataQueue.empty()) return false;
        value = mDataQueue.front();
        mDataQueue.pop();
        return true;
    }
    
    void waitAndPop(T& value)
    {
        std::unique_lock<std::mutex> lg(mLock);
        mDataCond.wait(lg, [this]() { return !mDataQueue.empty();});
        value = mDataQueue.front();
        mDataQueue.pop();
    }
    
    void push(T val)
    {
        std::lock_guard<std::mutex> lg(mLock);
        mDataQueue.push(val);
        mDataCond.notify_one();
    }
    
    bool empty()
    {
        std::lock_guard<std::mutex> lg(mLock);
        return mDataQueue.empty();
    }
    
    
private:
    std::queue<T> mDataQueue;
    mutable std::mutex mLock;
    std::condition_variable mDataCond;
};
