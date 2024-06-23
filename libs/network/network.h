#pragma once

#include <string>

namespace libs
{
    namespace network
    {
        namespace server
        {
            class Server
            {
            public:
                Server() = delete;
                Server(const std::string &address, const int port);
                ~Server();

                bool run();

            private:
                const std::string address_;
                const int port_;
            };
        }
    }
}