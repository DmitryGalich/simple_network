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
        std::string input;
        getline(std::cin, input);

        if (input == "q" ||
            input == "Q" ||
            input == "c" ||
            input == "C")
        {
            server.stop();
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Incorrect number of args: " << argc << std::endl;
        std::cerr << "Must be 1:" << std::endl;
        std::cerr << "1 - server port (int)" << std::endl;
        std::cerr << "Example: ./binary 8080" << std::endl;
        return -1;
    }

    int serverPort = 0;
    try
    {
        serverPort = std::stoi(argv[1]);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    try
    {
        const std::string kAddress("127.0.0.1");
        const std::string kLogFilePath("log.txt");

        if (!libs::logger::Logger::instance()->init(kLogFilePath))
        {
            std::cerr << "Can't init logger and saving log to file: " << kLogFilePath << std::endl;
            return -1;
        }

        libs::network::server::Server server([&](const std::string &message)
                                             { LOG(message); });

        std::future<void> user_command_future = std::async(&wait_for_user_command, std::ref(server));
        LOG("Input \'q\' to quit");

        if (!server.start({kAddress, serverPort, 500}))
        {
            LOG("Fail of running server: " + kAddress + ":" + std::to_string(serverPort));
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
