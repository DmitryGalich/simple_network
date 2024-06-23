#include "logger.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>
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

    bool writeToFile(const std::string &filePath, const std::string &message)
    {
        std::ofstream f;
        f.open(filePath, std::ios::app);
        if (!f.is_open())
            return false;

        f << message << '\n';
        f.close();

        return true;
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

        Logger::Logger() : running_(false)
        {
            workerThread_ = std::thread(&Logger::doLog, this);
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

        bool Logger::init(const std::string &filePath)
        {
            Logger::instance()->filePath_ = filePath;

            if (writeToFile(filePath_, "Init message"))
                return true;

            Logger::instance()->filePath_ = "";
            std::cerr << "Error while creating file: " << filePath << std::endl;
            return false;
        }

        void Logger::signalToLog(const std::string &message)
        {
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                messageQueue_.push("[" + getCurrentTime() + "] " + message);
            }
            condition_.notify_one();
        }

        void Logger::doLog()
        {
            running_.store(true);

            while (running_.load() || !messageQueue_.empty())
            {
                std::unique_lock<std::mutex> queueLock(queueMutex_);
                condition_.wait(queueLock, [this]
                                { return !messageQueue_.empty() || !running_.load(); });

                while (!messageQueue_.empty())
                {
                    std::string message = messageQueue_.front();
                    messageQueue_.pop();
                    queueLock.unlock();

                    std::cout << message << std::endl;

                    if (!writeToFile(filePath_, message))
                        std::cerr << "Error while writing to file: " << filePath_ << std::endl;

                    queueLock.lock();
                }
            }
        }
    }
}
