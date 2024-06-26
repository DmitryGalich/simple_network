#include <iostream>
#include <functional>

#include "logger.h"
#include "network.h"

#include <coroutine>
#include <future>
#include <thread>
#include <atomic>

void wait_for_user_command(libs::network::client::Client &client)
{
    while (true)
    {
        std::string input;
        getline(std::cin, input);

        if (input == "q" ||
            input == "Q" ||
            input == "c" ||
            input == "C")
        {
            client.stop();
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

        libs::network::client::Client client([&](const std::string &message)
                                             { LOG(message); });

        std::future<void> user_command_future = std::async(&wait_for_user_command, std::ref(client));
        LOG("Input \'q\' to quit");

        if (!client.start({"client", kAddress, port}))
        {
            LOG("Can't run client: " + kAddress + ":" + std::to_string(port));
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
