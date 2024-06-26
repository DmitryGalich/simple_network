#include "network.h"

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <atomic>
#include <thread>

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

                ServerImpl(const ServerImpl &) = default;
                ServerImpl &operator=(const ServerImpl &) = default;

                ServerImpl(const ServerImpl &&) = delete;
                ServerImpl &operator=(const ServerImpl &&) = delete;

                ServerImpl(std::function<void(const std::string &)> logCallback) : logCallback_(logCallback)
                {
                }
                ~ServerImpl()
                {
                    stop();
                }

                bool start(const Server::Config &config)
                {
                    if (isRunning_.load())
                    {
                        logCallback_("Server already started");
                        return false;
                    }

                    config_ = config;

                    if (!configure())
                        return false;

                    logCallback_("Server configured on port: " + std::to_string(config_.port_));

                    if (!runListeningCycle())
                        return false;

                    return true;
                }
                void stop()
                {
                    if (!isRunning_.load())
                        return;

                    isRunning_.store(false);
                }

            private:
                bool configure()
                {

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

                    if (bind(serverSocketFD_, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
                    {
                        logCallback_("Failed to bind socket");
                        closeConnection();
                        return false;
                    }

                    if (listen(serverSocketFD_, config_.maxClients_) < 0)
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

                    event_.events = EPOLLIN;
                    event_.data.fd = serverSocketFD_;
                    if (epoll_ctl(epollFD_, EPOLL_CTL_ADD, serverSocketFD_, &event_) == -1)
                    {
                        logCallback_("Failed to add server socket to epoll instance");
                        closeConnection();
                        return false;
                    }

                    events_ = std::make_unique<struct epoll_event[]>(config_.maxEvents_);
                    if (!events_)
                    {
                        logCallback_("Failed to create epoll events array");
                        closeConnection();
                        return false;
                    }

                    return true;
                };

                bool runListeningCycle()
                {
                    auto handleClient = [&](int clientFd)
                    {
                        char buffer[1024];

                        while (true)
                        {
                            int bytesRead = read(clientFd, buffer, sizeof(buffer));
                            if (bytesRead <= 0)
                                break;
                        }

                        logCallback_(buffer);

                        close(clientFd);
                    };
                    // ==================

                    isRunning_.store(true);

                    while (isRunning_.load())
                    {
                        int numEvents = epoll_wait(epollFD_, events_.get(), config_.maxEvents_, config_.waitingTimeout_);
                        if (numEvents == -1)
                        {
                            logCallback_("numEvents == -1");
                            continue;
                        }

                        for (int i = 0; i < numEvents; ++i)
                        {
                            if (events_[i].data.fd == serverSocketFD_)
                            {
                                struct sockaddr_in clientAddress;
                                socklen_t clientAddressLength = sizeof(clientAddress);
                                int clientFd = accept(serverSocketFD_, (struct sockaddr *)&clientAddress, &clientAddressLength);
                                if (clientFd == -1)
                                {
                                    logCallback_("Failed to accept client connection");
                                    continue;
                                }

                                event_.events = EPOLLIN;
                                event_.data.fd = clientFd;
                                if (epoll_ctl(epollFD_, EPOLL_CTL_ADD, clientFd, &event_) == -1)
                                {
                                    logCallback_("Failed to add client socket to epoll instance");
                                    close(clientFd);
                                    continue;
                                }

                                std::thread clientThread(handleClient, clientFd);
                                clientThread.detach();
                            }
                            else
                            {
                                int clientFd = events_[i].data.fd;
                                std::thread clientThread(handleClient, clientFd);
                                clientThread.detach();
                            }
                        }
                    }

                    closeConnection();
                    return true;
                }

                void closeConnection()
                {
                    events_.reset();

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

                struct epoll_event event_;
                std::unique_ptr<struct epoll_event[]> events_;

                std::function<void(const std::string &)> logCallback_;

                std::atomic<bool> isRunning_{false};
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