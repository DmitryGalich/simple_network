#include "logger.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace
{
    std::string getCurrentTime()
    {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm now_tm = *std::localtime(&now_time_t);

        std::ostringstream oss;
        oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setw(3) << std::setfill('0') << now_ms.count();

        return oss.str();
    }
}

namespace libs
{
    namespace logger
    {
        std::shared_ptr<Logger> Logger::instance()
        {
            static std::shared_ptr<Logger> instance(new Logger);
            return instance;
        }

        Logger::Logger() : running_(true)
        {
            workerThread_ = std::thread(&Logger::writeToFile, this);
        }

        Logger::~Logger()
        {
            running_.store(false);
            condition_.notify_all();
            if (workerThread_.joinable())
            {
                workerThread_.join();
            }
        }

        void Logger::init(const std::string &filePath)
        {
            Logger::instance()->filePath_ = filePath;
        }

        void Logger::print(const std::string &message)
        {
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                messageQueue_.push("[" + getCurrentTime() + "] " + message);
            }
            condition_.notify_one();
        }

        void Logger::writeToFile()
        {
            while (running_.load() || !messageQueue_.empty())
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                condition_.wait(lock, [this]
                                { return !messageQueue_.empty() || !running_.load(); });

                while (!messageQueue_.empty())
                {
                    std::string message = messageQueue_.front();
                    messageQueue_.pop();
                    lock.unlock();

                    std::cout << message << std::endl;

                    std::ofstream f(filePath_, std::ios::app);
                    if (f.is_open())
                    {
                        f << message << '\n';
                        f.close();
                    }
                    else
                    {
                        std::cerr << "Error while writing to file: " << filePath_ << std::endl;
                    }

                    lock.lock();
                }
            }
        }
    }
}
