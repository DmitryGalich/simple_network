#include "network.h"

#include <arpa/inet.h>
#include <cstring>
#include <atomic>
#include <thread>
#include <iomanip>
#include <string>

namespace
{
    std::string getCurrentTime()
    {
        auto now = std::chrono::system_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S.") << std::setw(3) << std::setfill('0') << microseconds;
        return ss.str();
    }
}

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

                    isRunning_.store(true);

                    while (isRunning_.load())
                    {
                        if (createAndConnect())
                            communicateWithServer();

                        closeConnection();
                        std::this_thread::sleep_for(std::chrono::seconds(config_.reconnectingTimeoutSeconds_));
                    }

                    return true;
                }

                void stop()
                {
                    if (!isRunning_.load())
                        return;

                    isRunning_.store(false);
                }

            private:
                bool createAndConnect()
                {
                    clientSocketFD_ = socket(AF_INET, SOCK_STREAM, 0);
                    if (clientSocketFD_ == -1)
                    {
                        logCallback_("Failed to create socket");
                        return false;
                    }

                    sockaddr_in server_addr;
                    memset(&server_addr, 0, sizeof(server_addr));
                    server_addr.sin_family = AF_INET;
                    server_addr.sin_port = htons(config_.port_);

                    if (inet_pton(AF_INET, config_.address_.c_str(), &server_addr.sin_addr) <= 0)
                    {
                        logCallback_("Failed to bind to address");
                        return false;
                    }

                    if (connect(clientSocketFD_, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
                    {
                        logCallback_("Failed to connect");
                        return false;
                    }

                    return true;
                }

                void communicateWithServer()
                {
                    const std::string kMessage("[" + getCurrentTime() + "] \"" + config_.title_ + "\"");

                    if (send(clientSocketFD_, kMessage.c_str(), kMessage.size(), 0) == -1)
                    {
                        logCallback_("Failed to send to server");
                    }
                }

                void closeConnection()
                {
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