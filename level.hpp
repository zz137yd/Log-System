/*
    1、定义枚举类, 枚举出日志等级
    2、提供转换接口: 将枚举转换为对应字符串
*/

#ifndef __M_LEVEL_H__
#define __M_LEVEL_H__

#include <iostream>
#include <string>

namespace Logs
{
    class LogLevel
    {
    public:
        enum class value
        {
            Unknown = 0,
            Debug,
            Info,
            Warn,
            Error,
            Fatal,
            OFF
        };
        
        //放在静态区, 这样就不需要非得用对象才能访问了, 就可以写Logs::LogLevel::toString
        static const char* toString(LogLevel::value level)
        {
            switch(level)
            {
                //适用于微软的编译器
                /*#define TOSTRING(name) #name
                case value::Debug: return TOSTRING(Debug);
                case value::Info: return TOSTRING(Info);
                case value::Warn: return TOSTRING(Warn);
                case value::Error: return TOSTRING(Error);
                case value::Fatal: return TOSTRING(Fatal);
                case value::OFF: return TOSTRING(OFF);
                #undef TOSTRING*/
                
                //比较清晰, 这里不需要别的操作
                case value::Debug: return "Debug";
                case value::Info: return "Info";
                case value::Warn: return "Warn";
                case value::Error: return "Error";
                case value::Fatal: return "Fatal";
                case value::OFF: return "OFF";
                default: return "UnKnown";
            }
            return "UnKnown";
        }
    };
}

#endif