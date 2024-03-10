/*Completing logger module:
    1、Logger base class
    2、Different derived classes (synchronous and asynchronous logger classes)
*/

#ifndef __M_LOG_H__
#define __M_LOG_H__

#include "format.hpp"
#include "sink.hpp"
#include "looper.hpp"
#include <atomic>
#include <mutex>
#include <cstdarg>
#include <unordered_map>
#include <type_traits>
#include <unistd.h>

namespace Logs
{
    class SyncLogger;
    class AsyncLogger;

    class Logger
    {
    public:
        enum class Type
        { 
            LOGGER_SYNC = 0,
            LOGGER_ASYNC
        };

        using ptr = std::shared_ptr<Logger>;

        Logger(const std::string& logger_name,
               Formatter::ptr formatter,
               std::vector<LogSink::ptr>& sinks,
               LogLevel::value level = LogLevel::value::Info) : _logger_name(logger_name),
                                                                 _formatter(formatter),
                                                                 _sinks(sinks.begin(), sinks.end()),
                                                                 _level(level) {}

        //A reference modified by const, so that it can't be changed externally, or std::string without &
        const std::string& name() {return _logger_name; }
        LogLevel::value loggerLevel() {return _level; }

        //Construct the log message object by passing in parameters, format the log, and finally sink
        void debug(const char* file, size_t line, const char* fmt, ...)
        {
            //1、Determine whether the current log reaches the output level
            if(LogLevel::value::Debug < _level) {return ;}

            //2、Organize the fmt formatted string and variable parameters into string, and obtain log information string
            //Format variable parameters
            va_list ap;
            va_start(ap, fmt);
            log(LogLevel::value::Debug, file, line, fmt, ap);
            va_end(ap);
        }

        void info(const char* file, size_t line, const char* fmt, ...)
        {
            if(LogLevel::value::Info < _level) {return ;}
            va_list ap;
            va_start(ap, fmt);
            log(LogLevel::value::Info, file, line, fmt, ap);
            va_end(ap);
        }

        void warn(const char* file, size_t line, const char* fmt, ...)
        {
            if(LogLevel::value::Warn < _level) {return ;}
            va_list ap;
            va_start(ap, fmt);
            log(LogLevel::value::Warn, file, line, fmt, ap);
            va_end(ap);
        }

        void error(const char* file, size_t line, const char* fmt, ...)
        {
            if(LogLevel::value::Error < _level) {return ;}
            va_list ap;
            va_start(ap, fmt);
            log(LogLevel::value::Error, file, line, fmt, ap);
            va_end(ap);
        }

        void fatal(const char* file, size_t line, const char* fmt, ...)
        {
            if(LogLevel::value::Fatal < _level) {return ;}
            va_list ap;
            va_start(ap, fmt);
            log(LogLevel::value::Fatal, file, line, fmt, ap);
            va_end(ap);
        }
    protected:
        void log(LogLevel::value level, const char* file, size_t line, const char* fmt, va_list ap)
        {
            char* buf;
            std::string msg;
            int len = vasprintf(&buf, fmt, ap);
            if(len < 0) msg = "Failed to format log message! ";
            else
            {
                msg.assign(buf, len);
                free(buf);
            }
            //3、Construct log message object
            LogMsg lm(level, line, file, _logger_name, msg);
            //4、Get the formatted string
            std::stringstream ss;
            _formatter->format(ss, lm);
            //5、Log sink
            logIt(ss.str());
        }

        virtual void logIt(const std::string& msg) = 0;
    protected:
        std::mutex _mutex;//Ensure the thread safety of log sink
        std::string _logger_name;
        Formatter::ptr _formatter;//The Formatter class in format.hpp uses smart pointer
        std::vector<LogSink::ptr> _sinks;//Sink
        std::atomic<LogLevel::value> _level;//Restriction level, only atomic access in multi-threads can avoid lock conflicts, etc
    };

    //Synchronous logger
    class SyncLogger : public Logger
    {
    public:
        using ptr = std::shared_ptr<SyncLogger>;

        SyncLogger(const std::string& logger_name,
                   Formatter::ptr formatter,
                   std::vector<LogSink::ptr>& sinks,
                   LogLevel::value level = LogLevel::value::Info) : Logger(logger_name, formatter, sinks, level)
        {
            std::cout << LogLevel::toString(level) << " Synchronous logger: " << _logger_name<< " created successfully...\n" << std::endl;
        }

    protected:
        //Sink the log through the sink module handle
        virtual void logIt(const std::string& msg)
        {
            //Automatically lock and automatically unlock when lock destroyed
            std::unique_lock<std::mutex> lock(_mutex);
            if (_sinks.empty()) return ;
            for (auto &sink : _sinks)
            {
                sink->log(msg.c_str(), msg.size());
            }
        }
    };

    class AsyncLogger : public Logger
    {
    public:
        using ptr = std::shared_ptr<AsyncLogger>;

        AsyncLogger(const std::string& logger_name,
                    Formatter::ptr formatter,
                    std::vector<LogSink::ptr>& sinks,
                    LogLevel::value level = LogLevel::value::Debug)
            : Logger(logger_name, formatter, sinks, level)
            , _looper(std::make_shared<AsyncLooper>(std::bind(&AsyncLogger::backendLogIt, this, std::placeholders::_1)))
        {
            std::cout << LogLevel::toString(level) << " Asynchronous logger: " << name() << " created successfully...\n" << std::endl;
        }//Use bind, because backendLogIt also comes with a this pointer
        //After using bind, there is only 1, which means that backendLogIt has been bound, so just pass one parameter instead of this

    protected:
        //Write data to buffer
        void logIt(const std::string& msg) {_looper->push(msg); }

        void backendLogIt(Buffer &msg)
        {
            if (_sinks.empty()) return;
            for (auto &sink : _sinks) sink->log(msg.begin(), msg.readAbleSize());
        }

    private:
        AsyncLooper::ptr _looper;
    };

    // Building a logger using builder pattern
    // 1、Abstract a logger builder class
    // 2、Set the logger type and put the creation of different types of loggers into the same logger builder class
    class Builder
    {
    public:
        using ptr = std::shared_ptr<Builder>;

        Builder()
            : _logger_type(Logger::Type::LOGGER_SYNC), _level(LogLevel::value::Info)
        {}

        void buildLoggerType(Logger::Type type) { _logger_type = type; }
        void buildLoggerName(const std::string& name) { _logger_name = name; }
        void buildLoggerLevel(LogLevel::value level) { _level = level; }
        void buildFormatter(const std::string& pattern) { _formatter = std::make_shared<Formatter>(pattern); }
        void buildFormatter(const Formatter::ptr& formatter) { _formatter = formatter; }
        /*void changeLooperType()
        {
            if (_looper_type == AsyncType::ASYNC_UNSAFE)
                _looper_type = AsyncType::ASYNC_SAFE;
            else _looper_type = AsyncType::ASYNC_UNSAFE;
        }*/

        template <typename SinkType, typename... Args>
        void buildSink(Args &&...args)
        {
            auto psink = SinkFactory::create<SinkType>(std::forward<Args>(args)...);
            _sinks.push_back(psink);
        }

        virtual Logger::ptr build() = 0;

    protected:
        Logger::Type _logger_type;
        std::string _logger_name;//Find the logger by _logger_name
        LogLevel::value _level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;
    };

    //3、Derive specific builder classes, including local and global logger builders
    //The global logger adds a global singleton manager and adds logger to global manager
    //Local logger
    class LocalLoggerBuilder : public Builder
    {
    public:
        virtual Logger::ptr build()
        {
            if(_logger_name.empty())
            {
                std::cout << "Logger name can't be empty! ";
                abort();
            }
            if(_formatter.get() == nullptr) 
            {
                std::cout << "Current logger: " << _logger_name << " doesn't detect log format, default is [ %d{%H:%M:%S}%T%t%T[%p]%T[%c]%T%f:%l%T%m%n ]! \n";
                _formatter = std::make_shared<Formatter>(); 
            }
            if(_sinks.empty()) 
            {
                std::cout << "Current logger: " << _logger_name << " doesn't detect the sink direction, and default setting is standard output! \n";
                buildSink<StdoutSink>(); 
            }
            Logger::ptr lp;
            if(_logger_type == Logger::Type::LOGGER_ASYNC)
                lp = std::make_shared<AsyncLogger>(_logger_name, _formatter, _sinks, _level);
            else
                lp = std::make_shared<SyncLogger>(_logger_name, _formatter, _sinks, _level);
            return lp;
        }
    };

    class LoggerManager
    {
    public:
        static LoggerManager& getInstance()
        {
            //After C++11, the compiler implements thread safety while compiling for static local variables
            //Before the static variable is constructed, other threads can only block and wait
            static LoggerManager eton;
            return eton;
        }

        LoggerManager(const LoggerManager&) = delete;
        LoggerManager &operator=(const LoggerManager&) = delete;

        bool hasLogger(const std::string& name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _loggers.find(name);
            if(it == _loggers.end()) return false;
            return true;
        }

        void addLogger(const std::string& name, Logger::ptr& logger)//Add logger
        {
            //When using the global log manager to create a singleton object
            //A synchronous logger named root has been created and saved with _root_logger
            //Even if the root name is passed when calling the add function, there is no problem
            //Because it will be added to _loggers, it has nothing to do with _root_logger
            if(hasLogger(logger->name())) return ;
            std::unique_lock<std::mutex> lock(_mutex);
            _loggers.insert(std::make_pair(name, logger));
        }

        Logger::ptr getLogger(const std::string& name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _loggers.find(name);
            if(it == _loggers.end()) return Logger::ptr();
            return it->second;
        }

        Logger::ptr rootLogger() 
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _root_logger; 
        }
    private:
        LoggerManager()
        {
            //This sentence cannot create a global manager because it will cause an infinite loop
            //Look at this global sentence: LoggerManager::getInstance().addLogger(logger);
            std::unique_ptr<LocalLoggerBuilder> builder(new LocalLoggerBuilder());
            //LocalLoggerBuilder's build function creates other parameters, so only the name is needed here
            builder->buildLoggerName("root");
            builder->buildLoggerType(Logger::Type::LOGGER_SYNC);
            std::cout << "\nBy default, a synchronization logger is created with the name root and level Info \n\n";
            _root_logger = builder->build();
            //To prevent someone from intentionally using the default logger, here is the following sentence:
            _loggers.insert(std::make_pair("root", _root_logger));
        }
    private:
        std::mutex _mutex;
        Logger::ptr _root_logger;
        std::unordered_map<std::string, Logger::ptr> _loggers;//Easy to find
    };

    //Global is to add a function based on local: add the logger to singleton object
    class GlobalLoggerBuilder : public Builder
    {
    public:
        Logger::ptr build() override
        {
            if(_logger_name.empty())
            {
                std::cout << "Logger name can't be empty! ";
                abort();
            }
            if(_formatter.get() == nullptr) 
            {
                std::cout << "Current logger: " << _logger_name << " doesn't detect log format, default is [ %d{%H:%M:%S}%T%t%T[%p]%T[%c]%T%f:%l%T%m%n ]! \n";
                _formatter = std::make_shared<Formatter>(); 
            }
            if(_sinks.empty()) 
            {
                std::cout << "Current logger: " << _logger_name << " doesn't detect the sink direction, and default setting is standard output! \n";
                buildSink<StdoutSink>(); 
            }
            Logger::ptr lp;
            if(_logger_type == Logger::Type::LOGGER_ASYNC)
                lp = std::make_shared<AsyncLogger>(_logger_name, _formatter, _sinks, _level);
            else
                lp = std::make_shared<SyncLogger>(_logger_name, _formatter, _sinks, _level);
            LoggerManager::getInstance().addLogger(_logger_name, lp);
            return lp;
        }
    };
}

#endif