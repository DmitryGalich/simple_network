#include "network.h"

namespace libs
{
    namespace network
    {
        namespace server
        {
            Server::Server(const std::string &address, const int port) : address_(address), port_(port) {}

            Server::~Server() {}

            bool Server::run()
            {
                return true;
            }

        }
    }
}