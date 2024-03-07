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
        time_t _ctime;//日志产生的时间戳
        size_t _line;//行号
        std::thread::id _tid;//线程ID
        std::string _file;//源码文件名
        std::string _name;//日志器名称
        std::string _payload;//有效载荷
        LogLevel::value _level;//日志等级

        LogMsg(LogLevel::value level,
               size_t line,
               const std::string file,
               const std::string& name,
               const std::string& payload) : _ctime(LogUtil::Date::now()), _level(level), _line(line),
                                              _tid(std::this_thread::get_id()), _file(file), _name(name), _payload(payload) {}
    };

    //util.hpp中, Date的now函数返回的是time_t类型, 和这里的_ctime相匹配
}

#endif
