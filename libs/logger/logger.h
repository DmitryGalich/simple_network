#pragma once

#include <memory>
#include <atomic>
#include <queue>
#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>

#define LOG(message) libs::logger::Logger::instance()->print(message)

namespace libs
{
    namespace logger
    {
        class Logger
        {
        public:
            static std::shared_ptr<Logger> instance();

            void init(const std::string &filePath);
            void print(const std::string &message);
            ~Logger();

        private:
            Logger();
            void writeToFile();

            std::string filePath_;
            std::atomic<bool> running_;
            std::queue<std::string> messageQueue_;
            std::thread workerThread_;
            std::mutex queueMutex_;
            std::condition_variable condition_;
        };
    }
}