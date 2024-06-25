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
        namespace client
        {

            class Client::ClientImpl
            {

            public:
                ClientImpl() = delete;
                ClientImpl(std::function<void(const std::string &)> logCallback) : logCallback_(logCallback)
                {
                }
                ~ClientImpl()
                {
                    stop();
                }

                bool start(const Client::Config &config)
                {
                    if (isRunning_.load())
                    {
                        logCallback_("Client already started");
                        return false;
                    }

                    config_ = config;
                    if (!configure())
                        return false;

                    logCallback_("Client configured on port: " + std::to_string(config_.port_));

                    if (!runListeningCycle())
                        return false;

                    return true;
                }
                void stop()
                {
                    if (!isRunning_.load())
                        return;

                    logCallback_("Stopping client");
                    isRunning_.store(false);
                }

            private:
                bool configure()
                {
                    logCallback_("Configuring client");

                    clientSocketFD_ = socket(AF_INET, SOCK_STREAM, 0);
                    if (clientSocketFD_ == kIncorrectSocketValue_)
                    {
                        logCallback_("Failed to create socket");
                        return false;
                    }

                    struct sockaddr_in clientAddress;
                    clientAddress.sin_family = AF_INET;
                    clientAddress.sin_addr.s_addr = inet_addr(config_.address_.c_str());
                    clientAddress.sin_port = htons(config_.port_);

                    int status = connect(clientSocketFD_,
                                         (sockaddr *)&clientAddress, sizeof(clientAddress));
                    if (status < 0)
                    {
                        return -1;
                    }

                    // if (bind(clientSocketFD_, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) == -1)
                    // {
                    //     logCallback_("Failed to bind socket");
                    //     closeConnection();
                    //     return false;
                    // }

                    // if (listen(clientSocketFD_, config_.maxClients_) == -1)
                    // {
                    //     logCallback_("Failed to listen socket");
                    //     closeConnection();
                    //     return false;
                    // }

                    // epollFD_ = epoll_create1(0);
                    // if (epollFD_ == kIncorrectSocketValue_)
                    // {
                    //     logCallback_("Failed to create epoll instance");
                    //     closeConnection();
                    //     return false;
                    // }

                    // event.events = EPOLLIN;
                    // event.data.fd = clientSocketFD_;
                    // if (epoll_ctl(epollFD_, EPOLL_CTL_ADD, clientSocketFD_, &event) == -1)
                    // {
                    //     logCallback_("Failed to add client socket to epoll instance");
                    //     closeConnection();
                    //     return false;
                    // }

                    return true;
                };

                bool runListeningCycle()
                {
                    isRunning_.store(true);

                    logCallback_("Listening cycle started");

                    while (isRunning_.load())
                    {
                    }

                    logCallback_("Escape from listening cycle");

                    closeConnection();
                    return true;
                }

                void closeConnection()
                {
                    logCallback_("Closing connection");

                    if (clientSocketFD_ != kIncorrectSocketValue_)
                        close(clientSocketFD_);

                    clientSocketFD_ = kIncorrectSocketValue_;
                }

            private:
                Client::Config config_;

                const int kIncorrectSocketValue_{-1};
                int clientSocketFD_{kIncorrectSocketValue_};

                std::function<void(const std::string &)> logCallback_;

                std::atomic<bool> isRunning_{false};
            };

            Client::Client(std::function<void(const std::string &)> logCallback) : clientImpl_(std::make_unique<Client::ClientImpl>(logCallback)) {}

            Client::~Client()
            {
                clientImpl_->stop();
            }

            bool Client::start(const Config &config)
            {
                if (!clientImpl_)
                    throw std::runtime_error("Implementation is not created");

                return clientImpl_->start(config);
            }

            void Client::stop()
            {
                if (!clientImpl_)
                    throw std::runtime_error("Implementation is not created");

                return clientImpl_->stop();
            }
        }
    }
}