/*完成日志器模块:
    1、日志器基类
    2、不同派生类（同、异步日志器类）
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

        //const修饰的引用, 这样外部无法更改, 或者也可以std::string, 不加&
        const std::string& name() {return _logger_name; }
        LogLevel::value loggerLevel() {return _level; }

        //通过传入的参数构造出一个日志消息对象, 进行日志格式化, 最终落地
        void debug(const char* file, size_t line, const char* fmt, ...)
        {
            //1、判断当前日志是否达到输出等级
            if(LogLevel::value::Debug < _level) {return ;}

            //2、对fmt格式化字符串和不定参数进行字符串组织, 得到的日志信息的字符串
            //对不定参进行格式化
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
            if(len < 0) msg = "格式化日志消息失败! ";
            else
            {
                msg.assign(buf, len);
                free(buf);
            }
            //3、构造日志消息对象
            LogMsg lm(level, line, file, _logger_name, msg);
            //4、得到格式化后的字符串
            std::stringstream ss;
            _formatter->format(ss, lm);
            //5、日志落地
            logIt(ss.str());
        }

        virtual void logIt(const std::string& msg) = 0;
    protected:
        std::mutex _mutex;//保证日志落地的线程安全
        std::string _logger_name;
        Formatter::ptr _formatter;//format.hpp中Formatter类加上智能指针
        std::vector<LogSink::ptr> _sinks;//落地
        std::atomic<LogLevel::value> _level;//限制等级, 在多线程中只能原子访问而避免了加锁冲突等
    };

    //同步日志器
    class SyncLogger : public Logger
    {
    public:
        using ptr = std::shared_ptr<SyncLogger>;

        SyncLogger(const std::string& logger_name,
                   Formatter::ptr formatter,
                   std::vector<LogSink::ptr>& sinks,
                   LogLevel::value level = LogLevel::value::Info) : Logger(logger_name, formatter, sinks, level)
        {
            std::cout << LogLevel::toString(level) << " 同步日志器: " << _logger_name<< " 创建成功...\n" << std::endl;
        }

    protected:
        //将日志通过落地模块句柄来落地
        virtual void logIt(const std::string& msg)
        {
            //自动加锁, 等到lock释放时就自动解锁
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
            std::cout << LogLevel::toString(level) << " 异步日志器: " << name() << " 创建成功...\n" << std::endl;
        }//用绑定, 因为backendLogIt还会自带一个this指针, 用bind后, 只有1, 那就说明backendLogIt已经被绑定, 所以只传一个参数而不传this即可

    protected:
        //数据写入缓冲区
        void logIt(const std::string& msg) {_looper->push(msg); }

        void backendLogIt(Buffer &msg)
        {
            if (_sinks.empty()) return;
            for (auto &sink : _sinks) sink->log(msg.begin(), msg.readAbleSize());
        }

    private:
        AsyncLooper::ptr _looper;
    };

    // 建造者模式建造日志器
    // 1、抽象一个日志器建造者类
    // 2、设置日志器类型，将不同类型的日志器的创建放到同一个日志器建造者类中完成
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
        std::string _logger_name;//日志器名称, 之后要通过名称找到日志器
        LogLevel::value _level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;
    };

    //3、派生出具体的建造者类，包括局部、全局日志器建造者，全局是添加了全局单例管理器，将日志器添加到全局管理器中
    //局部日志器
    class LocalLoggerBuilder : public Builder
    {
    public:
        virtual Logger::ptr build()
        {
            if(_logger_name.empty())
            {
                std::cout << "日志器名称不能为空! ";
                abort();
            }
            if(_formatter.get() == nullptr) 
            {
                std::cout << "当前日志器: " << _logger_name << " 未检测到日志格式，默认设置为[ %d{%H:%M:%S}%T%t%T[%p]%T[%c]%T%f:%l%T%m%n ]!\n";
                _formatter = std::make_shared<Formatter>(); 
            }
            if(_sinks.empty()) 
            {
                std::cout << "当前日志器: " << _logger_name << " 未检测到落地方向，默认设置为标准输出!\n";
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
            //C++11后, 针对静态局部变量, 编译器在编译的层面实现了线程安全
            //在静态变量没有构造完成之前, 其它线程只能阻塞等待
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

        void addLogger(const std::string& name, Logger::ptr& logger)//添加日志器
        {
            //当使用全局日志管理器创建单例对象时, 就已经创建了一个名为root的同步日志器, 用_root_logger来保存
            //即使调用add函数时也传了root名字也没问题, 因为它会被添加到_loggers, 和_root_logger没关系
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
            //这句不能创建全局是因为会造成死循环, 看全局的这句: LoggerManager::getInstance().addLogger(logger);
            std::unique_ptr<LocalLoggerBuilder> builder(new LocalLoggerBuilder());
            //LocalLoggerBuilder的build函数都创建了其它参数, 所以这里只需要名字
            builder->buildLoggerName("root");
            builder->buildLoggerType(Logger::Type::LOGGER_SYNC);
            std::cout << "\n默认创建一个同步日志器，名字为root，等级为Info\n\n";
            _root_logger = builder->build();
            //防止有故意要拿默认日志器的情况，所以有下句
            _loggers.insert(std::make_pair("root", _root_logger));
        }
    private:
        std::mutex _mutex;
        Logger::ptr _root_logger;
        std::unordered_map<std::string, Logger::ptr> _loggers;//方便查找
    };

    //全局是在局部基础上添加一个功能: 将日志器添加到单例对象中
    class GlobalLoggerBuilder : public Builder
    {
    public:
        Logger::ptr build() override
        {
            if(_logger_name.empty())
            {
                std::cout << "日志器名称不能为空! ";
                abort();
            }
            if(_formatter.get() == nullptr) 
            {
                std::cout << "当前日志器: " << _logger_name << " 未检测到日志格式，默认设置为[ %d{%H:%M:%S}%T%t%T[%p]%T[%c]%T%f:%l%T%m%n ]!\n";
                _formatter = std::make_shared<Formatter>(); 
            }
            if(_sinks.empty()) 
            {
                std::cout << "当前日志器: " << _logger_name << " 未检测到落地方向，默认设置为标准输出!\n";
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