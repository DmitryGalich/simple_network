#include <iostream>

#include "logger.h"
#include "network.h"

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

        libs::network::server::Server server;
        if (!server.start({kAddress, port}))
        {
            LOG("Can't run server: " + kAddress + ":" + std::to_string(port));
            return -1;
        }

        LOG("Programm correct closed");
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
