#pragma once

#include <memory>
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

        private:
            Logger() = default;

            bool saveToFile(const std::string &message);

        private:
            std::string filePath_;
            std::mutex mtx_;
        };
    }
}