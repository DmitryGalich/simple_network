#include "network.h"

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#include <atomic>
#include <thread>
#include <chrono>

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

                    logCallback_("Client configured on port: " + std::to_string(config_.port_));

                    isRunning_.store(true);

                    // Connecting cycle
                    while (isRunning_.load())
                    {
                        if (configure())
                        {
                            if (connect(clientSocketFD_, reinterpret_cast<struct sockaddr *>(serverAddr_.get()), sizeof(sockaddr_in)) == 0)
                            {
                                if (!send(clientSocketFD_, config_.title_.c_str(), config_.title_.size(), 0))
                                {
                                    logCallback_("Failed to send to server");
                                }
                            }
                            else
                            {
                                logCallback_("Failed to connect to server");
                            }
                        }
                        else
                        {
                            logCallback_("Failed to configure client");
                        }

                        closeConnection();
                        std::this_thread::sleep_for(std::chrono::seconds(config_.reconnectingTimeoutSeconds_));
                    }

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
                    if (clientSocketFD_ < 0)
                    {
                        logCallback_("Failed to create socket");
                        return false;
                    }

                    serverAddr_ = std::make_unique<sockaddr_in>();
                    memset(serverAddr_.get(), 0, sizeof(sockaddr_in));
                    serverAddr_->sin_family = AF_INET;
                    serverAddr_->sin_port = htons(config_.port_);

                    if (inet_pton(AF_INET, config_.address_.c_str(), &serverAddr_->sin_addr) <= 0)
                    {
                        logCallback_("Invalid address/ Address not supported");
                        return false;
                    }

                    return true;
                };

                void closeConnection()
                {
                    logCallback_("Closing connection");

                    serverAddr_.reset();

                    if (clientSocketFD_ != kIncorrectSocketValue_)
                        close(clientSocketFD_);

                    clientSocketFD_ = kIncorrectSocketValue_;
                }

            private:
                Client::Config config_;

                const int kIncorrectSocketValue_{-1};
                int clientSocketFD_{kIncorrectSocketValue_};

                std::function<void(const std::string &)> logCallback_;

                std::unique_ptr<sockaddr_in> serverAddr_;

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