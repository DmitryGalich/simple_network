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

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Incorrect number of args: " << argc << std::endl;
        std::cerr << "Must be 3:" << std::endl;
        std::cerr << "1 - client name (string)" << std::endl;
        std::cerr << "2 - server port (int)" << std::endl;
        std::cerr << "3 - reconnect timeout in seconds (int)" << std::endl;
        return -1;
    }

    const std::string kClientTitle = argv[1];

    int serverPort = 0;
    try
    {
        serverPort = std::stoi(argv[2]);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    int reconnectingTimeoutSec = 0;
    try
    {
        reconnectingTimeoutSec = std::stoi(argv[3]);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

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

        if (!client.start({kClientTitle, kAddress, serverPort, reconnectingTimeoutSec}))
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
