#include "network.h"

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <atomic>
#include <thread>

#include <iostream>
#include <cerrno>
#include <cstring>

#define MAX_EVENTS 10
#define READ_BUFFER_SIZE 1024

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

                    if (!createAndBind())
                    {
                        closeConnection();
                        return false;
                    }

                    if (!setupEpoll())
                    {
                        closeConnection();
                        return false;
                    }

                    runServer();

                    closeConnection();
                    return true;
                }

                void stop()
                {
                    if (!isRunning_.load())
                        return;

                    isRunning_.store(false);
                }

            private:
                bool createAndBind()
                {
                    serverSocketFD_ = socket(AF_INET, SOCK_STREAM, 0);
                    if (serverSocketFD_ == -1)
                    {
                        logCallback_("Failed to create socket");
                        return false;
                    }

                    sockaddr_in serverAddr;
                    memset(&serverAddr, 0, sizeof(serverAddr));
                    serverAddr.sin_family = AF_INET;
                    serverAddr.sin_port = htons(config_.port_);
                    if (inet_pton(AF_INET, config_.address_.c_str(), &serverAddr.sin_addr) == -1)
                    {
                        logCallback_("Invalid address/ Address not supported");
                        return false;
                    }

                    if (bind(serverSocketFD_, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
                    {
                        logCallback_("Failed to bind");
                        return false;
                    }

                    if (listen(serverSocketFD_, SOMAXCONN) == -1)
                    {
                        logCallback_("Failed to listen");
                        return false;
                    }

                    return true;
                }

                bool setupEpoll()
                {
                    epollFD_ = epoll_create1(0);
                    if (epollFD_ == -1)
                    {
                        logCallback_("Failed to create epoll");
                        return false;
                    }

                    epoll_event ev;
                    ev.events = EPOLLIN;
                    ev.data.fd = serverSocketFD_;
                    if (epoll_ctl(epollFD_, EPOLL_CTL_ADD, serverSocketFD_, &ev) == -1)
                    {
                        logCallback_("Failed to configure epoll");
                        return false;
                    }

                    return true;
                }

                void runServer()
                {
                    epoll_event events[MAX_EVENTS];
                    char readBuffer[READ_BUFFER_SIZE];

                    isRunning_.store(true);

                    while (isRunning_.load())
                    {
                        int n = epoll_wait(epollFD_, events, MAX_EVENTS, -1);
                        if (n == -1)
                        {
                            logCallback_("Fail of epoll_wait");
                            break;
                        }

                        for (int i = 0; i < n; ++i)
                        {
                            if (events[i].data.fd == serverSocketFD_)
                            {
                                handleNewConnection();
                            }
                            else
                            {
                                // Handle data from a client
                                int client_fd = events[i].data.fd;
                                int bytes_read = read(client_fd, readBuffer, READ_BUFFER_SIZE);
                                if (bytes_read == -1)
                                {
                                    perror("read");
                                    close(client_fd);
                                    continue;
                                }
                                else if (bytes_read == 0)
                                {
                                    std::cout << "Client disconnected" << std::endl;
                                    close(client_fd);
                                }
                                else
                                {
                                    std::cout << "Received: " << std::string(readBuffer, bytes_read) << std::endl;
                                }
                            }
                        }
                    }
                }

                bool setNonBlocking(int fd)
                {
                    int flags = fcntl(fd, F_GETFL, 0);
                    if (flags == -1)
                    {
                        logCallback_("Failed to get flags");
                        return false;
                    }

                    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
                    {
                        logCallback_("Failed to set flags");
                        return false;
                    }

                    return true;
                }

                void handleNewConnection()
                {
                    sockaddr_in clientAddr;
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    int clientFD = accept(serverSocketFD_, (sockaddr *)&clientAddr, &clientAddrLen);
                    if (clientFD == -1)
                    {
                        logCallback_("Failed to accept");
                        return;
                    }

                    if (!setNonBlocking(clientFD))
                    {
                        logCallback_("Failed to set nonblocking");
                        close(clientFD);
                        return;
                    }

                    epoll_event ev;
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = clientFD;

                    if (epoll_ctl(epollFD_, EPOLL_CTL_ADD, clientFD, &ev) == -1)
                    {
                        logCallback_("Failed to add new client");
                        close(clientFD);
                        return;
                    }

                    logCallback_("Accepted connection from " + std::string(inet_ntoa(clientAddr.sin_addr)) + ":" + std::to_string(ntohs(clientAddr.sin_port)));
                }

                void closeConnection()
                {
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