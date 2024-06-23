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
                enum class ClosingMode
                {
                    Socket = 0,
                    SocketAndEpoll
                };

            public:
                ServerImpl() = delete;
                ServerImpl(std::function<void(const std::string &)> logCallback) : logCallback_(logCallback) {}
                ~ServerImpl()
                {
                    stop();
                }

                bool start(const Server::Config &config)
                {
                    logCallback_("Starting server");

                    config_ = config;

                    if (isRunning_)
                        stop();

                    configure();

                    isRunning_ = true;
                    return true;
                }
                void stop()
                {
                    if (!isRunning_)
                        return;

                    logCallback_("Stopping server");

                    isRunning_ = false;
                }

            private:
                bool configure()
                {
                    logCallback_("Configuring server");

                    serverSocketFD_ = socket(AF_INET, SOCK_STREAM, 0);
                    if (serverSocketFD_ == -1)
                        return false;

                    struct sockaddr_in serverAddress;
                    serverAddress.sin_family = AF_INET;
                    serverAddress.sin_addr.s_addr = inet_addr(config_.address_.c_str());
                    serverAddress.sin_port = htons(config_.port_);

                    if (bind(serverSocketFD_, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
                    {
                        closeConnection();
                        return false;
                    }

                    return true;
                };

                void closeConnection(const ClosingMode mode = ClosingMode::Socket)
                {
                    switch (mode)
                    {
                    case ClosingMode::SocketAndEpoll:
                        close(epollFD_);
                        close(serverSocketFD_);
                        break;
                    case ClosingMode::Socket:
                        close(serverSocketFD_);
                        break;
                    default:
                    {
                    }
                    }
                }

            private:
                bool isRunning_{false};
                Server::Config config_;

                int serverSocketFD_{-1};
                int epollFD_{-1};

                std::function<void(const std::string &)> logCallback_;
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