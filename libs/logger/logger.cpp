#include "logger.h"

#include <string>
#include <iostream>
#include <fstream>

namespace libs
{
    namespace logger
    {
        std::shared_ptr<Logger> Logger::instance()
        {
            static std::shared_ptr<Logger> instance(new Logger);
            return instance;
        }

        void Logger::init(const std::string &filePath)
        {
            Logger::instance()->filePath_ = filePath;
        }

        void Logger::print(const std::string &message)
        {
            {
                std::lock_guard<std::mutex> lock(mtx_);
                std::cout << message << std::endl;
            }

            if (!Logger::instance()->saveToFile(message))
            {
                {
                    std::lock_guard<std::mutex> lock(mtx_);
                    std::cout << "Error while writing to file: " << Logger::instance()->filePath_ << std::endl;
                }
            }
        }

        bool Logger::saveToFile(const std::string &message)
        {
            if (filePath_.empty())
                return false;

            {
                std::lock_guard<std::mutex> lock(mtx_);

                std::ofstream f(filePath_, std::ios::app);
                if (!f.is_open())
                    return false;

                f << message;
                f << '\n';

                f.close();
            }

            return true;
        }
    }
}
