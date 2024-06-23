#include <iostream>
#include <functional>

#include "logger.h"
#include "network.h"

#include <coroutine>
#include <future>
#include <thread>
#include <atomic>

void wait_for_user_command(libs::network::server::Server &server)
{
    while (true)
    {
        if ((std::cin.get() == 'q') || (std::cin.get() == 'Q'))
        {
            server.stop();
            break;
        }
    }
}

int main()
{

    try
    {
        const std::string kAddress("127.0.0.1");
        int port = 8080;

        const std::string kLogFilePath("log.txt");

        if (!libs::logger::Logger::instance()->init(kLogFilePath))
        {
            std::cerr << "Can't init logger and saving log to file: " << kLogFilePath << std::endl;
            return -1;
        }

        libs::network::server::Server server([&](const std::string &message)
                                             { LOG(message); });

        std::future<void> user_command_future = std::async(&wait_for_user_command, std::ref(server));

        if (false)
            return -1;

        if (!server.start({kAddress, port, 10, 10}))
        {
            LOG("Can't run server: " + kAddress + ":" + std::to_string(port));
            return -1;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
