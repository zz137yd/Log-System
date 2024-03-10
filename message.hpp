#ifndef __M_MSG_H__
#define __M_MSG_H__

#include "util.hpp"
#include "level.hpp"
#include <thread>
#include <memory>

namespace Logs
{
    struct LogMsg
    {
        using ptr = std::shared_ptr<LogMsg>;
        time_t _ctime;//Timestamp of log generation
        size_t _line;//Line number
        std::thread::id _tid;//Thread ID
        std::string _file;//Source code file name
        std::string _name;//Logger name
        std::string _payload;//Payload
        LogLevel::value _level;//Log level

        LogMsg(LogLevel::value level,
               size_t line,
               const std::string file,
               const std::string& name,
               const std::string& payload) : _ctime(LogUtil::Date::now()), _level(level), _line(line),
                                              _tid(std::this_thread::get_id()), _file(file), _name(name), _payload(payload) {}
    };

    //In util.hpp, Date::now returns time_t type, which matches the _ctime here.
}

#endif
