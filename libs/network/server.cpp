#include "network.h"

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <atomic>

namespace libs
{
    namespace network
    {
        namespace server
        {

            class Server::ServerImpl
            {

            public:
                ServerImpl() = delete;
                ServerImpl(std::function<void(const std::string &)> logCallback) : logCallback_(logCallback) {}
                ~ServerImpl()
                {
                    stop();
                }

                bool start(const Server::Config &config)
                {
                    if (!isRunning_.load())
                    {
                        logCallback_("Server already started");
                        return false;
                    }

                    logCallback_("Starting server");

                    config_ = config;
                    if (!configure())
                        return false;

                    logCallback_("Server started. Listening on port: " + std::to_string(config_.port_));

                    if (!runListeningCycle())
                        return false;

                    return true;
                }
                void stop()
                {
                    if (!isRunning_.load())
                        return;

                    logCallback_("Stopping server");
                    isRunning_.store(false);
                }

            private:
                bool configure()
                {
                    logCallback_("Configuring server");

                    serverSocketFD_ = socket(AF_INET, SOCK_STREAM, 0);
                    if (serverSocketFD_ == kIncorrectSocketValue_)
                    {
                        logCallback_("Failed to create socket");
                        return false;
                    }

                    struct sockaddr_in serverAddress;
                    serverAddress.sin_family = AF_INET;
                    serverAddress.sin_addr.s_addr = inet_addr(config_.address_.c_str());
                    serverAddress.sin_port = htons(config_.port_);

                    if (bind(serverSocketFD_, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
                    {
                        logCallback_("Failed to bind socket");
                        closeConnection();
                        return false;
                    }

                    if (listen(serverSocketFD_, config_.maxClients_) == -1)
                    {
                        logCallback_("Failed to listen socket");
                        closeConnection();
                        return false;
                    }

                    epollFD_ = epoll_create1(0);
                    if (epollFD_ == kIncorrectSocketValue_)
                    {
                        logCallback_("Failed to create epoll instance");
                        closeConnection();
                        return false;
                    }

                    event.events = EPOLLIN;
                    event.data.fd = serverSocketFD_;
                    if (epoll_ctl(epollFD_, EPOLL_CTL_ADD, serverSocketFD_, &event) == -1)
                    {
                        logCallback_("Failed to add server socket to epoll instance");
                        closeConnection();
                        return false;
                    }

                    return true;
                };

                bool runListeningCycle()
                {
                    isRunning_.store(true);

                    while (isRunning_.load())
                    {
                        logCallback_("KEK");
                    }

                    closeConnection();
                    return true;
                }

                void closeConnection()
                {
                    logCallback_("Closing connection");

                    if (epollFD_ != kIncorrectSocketValue_)
                        close(epollFD_);

                    if (serverSocketFD_ != kIncorrectSocketValue_)
                        close(serverSocketFD_);

                    serverSocketFD_ = kIncorrectSocketValue_;
                    epollFD_ = kIncorrectSocketValue_;
                }

            private:
                Server::Config config_;

                const int kIncorrectSocketValue_{-1};
                int serverSocketFD_{kIncorrectSocketValue_};
                int epollFD_{kIncorrectSocketValue_};

                struct epoll_event event;
                std::unique_ptr<struct epoll_event[]> events_;

                std::function<void(const std::string &)> logCallback_;

                std::atomic<bool> isRunning_;
            };

            Server::Server(std::function<void(const std::string &)> logCallback) : serverImpl_(std::make_unique<Server::ServerImpl>(logCallback)) {}

            Server::~Server()
            {
                serverImpl_->stop();
            }

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