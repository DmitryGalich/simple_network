#include <iostream>
#include <thread>
#include <vector>

#include "logger.h"

void fun(int n)
{
    for (int i = 0; i < n; ++i)
    {
        LOG("KEK define");
    }
}

int main()
{
    libs::logger::Logger::instance()->init("log.txt");
    // LOG("KEK define");
    // LOG("KEK define1");

    const int kThreadsNum = 100;
    const int kCounts = 100;

    std::vector<std::thread> threads;
    threads.reserve(kThreadsNum);

    for (int i = 0; i < kThreadsNum; ++i)
    {
        threads.emplace_back(std::thread(fun, kCounts));
    }

    for (auto &thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }

    return 0;
}