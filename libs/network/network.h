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

                Server(const Server &) = default;
                Server &operator=(const Server &) = default;

                Server(const Server &&) = delete;
                Server &operator=(const Server &&) = delete;

                ~Server();

                bool start(const Config &config);
                void stop();

            private:
                class ServerImpl;
                std::unique_ptr<ServerImpl> serverImpl_;
            };
        }

        namespace client
        {
            class Client
            {
            public:
                struct Config
                {
                    std::string address_{"127.0.0.1"};
                    int port_{8080};
                    int reconnectingTimeoutMs_ = 500;
                };

                Client() = delete;
                Client(std::function<void(const std::string &)> logCallback);

                Client(const Client &) = default;
                Client &operator=(const Client &) = default;

                Client(const Client &&) = delete;
                Client &operator=(const Client &&) = delete;

                ~Client();

                bool start(const Config &config);
                void stop();

            private:
                class ClientImpl;
                std::unique_ptr<ClientImpl> clientImpl_;
            };
        }
    }
}