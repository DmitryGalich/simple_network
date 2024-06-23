#include "logger.h"

#include <thread>
#include <fstream>
#include <iostream>

void fun()
{
    for (int i = 0; i < 100; ++i)
    {
        LOG(std::to_string(i));
    }
}

int main()
{

    if (!libs::logger::Logger::instance()->init("log.txt"))
        return -1;

    LOG("KEK");

    std::thread funThread(fun);
    funThread.join();

    std::thread funThread1(fun);
    funThread1.join();

    return 0;
}
