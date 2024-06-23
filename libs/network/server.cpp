#include "network.h"

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace libs
{
    namespace network
    {
        namespace server
        {

            class Server::ServerImpl
            {
            public:
                ServerImpl() {}
                ~ServerImpl() {}

                bool start(const Server::Config &config)
                {
                    config_ = config;

                    if (isRunning_)
                        stop();

                    isRunning_ = true;
                    return true;
                }
                void stop()
                {
                    if (!isRunning_)
                        return;

                    isRunning_ = false;
                }

            private:
                bool isRunning_{false};
                Server::Config config_;

                int serverDescriptor_{0};
                int epollDescriptor_{0};
                // struct sockaddr_in serverAddress;
                // struct epoll_event event, events[MAX_EVENTS];
            };

            Server::Server() : serverImpl_(std::make_unique<Server::ServerImpl>()) {}

            Server::~Server() {}

            bool Server::start(const Config &config)
            {
                if (!serverImpl_)
                    throw std::runtime_error("Implementation is not created");

                return serverImpl_->start(config);
            }

            void Server::stop()
            {
                if (!serverImpl_)
                    throw std::runtime_error("Implementation is not created");

                return serverImpl_->stop();
            }
        }
    }
}