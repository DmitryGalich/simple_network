#pragma once

#include <string>
#include <memory>
#include <functional>

namespace libs
{
    namespace network
    {
        namespace server
        {
            class Server
            {
            public:
                struct Config
                {
                    std::string address_{"127.0.0.1"};
                    int port_{8080};
                    int maxEvents_ = 10;
                    int maxClients_ = 10;
                    int waitingTimeout_ = 100;
                };

                Server() = delete;
                Server(std::function<void(const std::string &)> logCallback);
                ~Server();

                bool start(const Config &config);
                void stop();

            private:
                class ServerImpl;
                std::unique_ptr<ServerImpl> serverImpl_;
            };
        }
    }
}