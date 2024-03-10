

#include "logger.hpp"

namespace Logs
{
    Logger::ptr getLogger(const std::string& name)
    {
        return Logs::LoggerManager::getInstance().getLogger(name);
    }

    Logger::ptr rootLogger()
    {
        return Logs::LoggerManager::getInstance().rootLogger();
    }

    #define DeBug(fmt, ...) debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define InFo(fmt, ...) info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define WaRn(fmt, ...) warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define ErrOr(fmt, ...) error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define FaTal(fmt, ...) fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

    #define DEBUG(fmt, ...) Logs::rootLogger()->DeBug(fmt, ##__VA_ARGS__)
    #define INFO(fmt, ...) Logs::rootLogger()->InFo(fmt, ##__VA_ARGS__)
    #define WARN(fmt, ...) Logs::rootLogger()->WaRn(fmt, ##__VA_ARGS__)
    #define ERROR(fmt, ...) Logs::rootLogger()->ErrOr(fmt, ##__VA_ARGS__)
    #define FATAL(fmt, ...) Logs::rootLogger()->FaTal(fmt, ##__VA_ARGS__)
}

#endif