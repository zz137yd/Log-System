/*
    1、Define the level class and enumerate the log levels
    2、Provides a conversion interface: Convert enumerations to corresponding strings
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
        
        //Put it in the static area, so we don't need to use an object to access it, and can write Logs::LogLevel::toString
        static const char* toString(LogLevel::value level)
        {
            switch(level)
            {
                //Applicable to Microsoft compiler
                /*#define TOSTRING(name) #name
                case value::Debug: return TOSTRING(Debug);
                case value::Info: return TOSTRING(Info);
                case value::Warn: return TOSTRING(Warn);
                case value::Error: return TOSTRING(Error);
                case value::Fatal: return TOSTRING(Fatal);
                case value::OFF: return TOSTRING(OFF);
                #undef TOSTRING*/
                
                //It’s relatively clear and no other operations are needed here.
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