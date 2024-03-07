#include "logs.h"

void test_log()
{
    //Time
    /*Logs::Logger::ptr logger = Logs::LoggerManager::getInstance().getLogger("async_logger");
    logger->debug(__FILE__, __LINE__, "%s", "测试日志d");
    logger->info(__FILE__, __LINE__, "%s", "测试日志i");
    logger->warn(__FILE__, __LINE__, "%s", "测试日志w");
    logger->error(__FILE__, __LINE__, "%s", "测试日志e");
    logger->fatal(__FILE__, __LINE__, "%s", "测试日志f");
    size_t count = 0;
    std::string str = "测试日志-";
    while(count < 470000)
    {
        std::string tmp = str + std::to_string(count++);
        logger->fatal(__FILE__, __LINE__, tmp.c_str());
    }*/
    /*DEBUG("%s", "测试日志");
    INFO("%s", "测试日志");
    WARN("%s", "测试日志");
    ERROR("%s", "测试日志");
    FATAL("%s", "测试日志");
    size_t count = 0;
    while(count < 470000)
    {
        FATAL("测试日志-%d", count++);
    }*/


    //Size
    /*size_t cursize = 0, count = 0;
    std::string str = "测试日志-";
    while(cursize < 1024 * 1024 * 10)//小于10MB就持续写入
    {
        std::string tmp = str + std::to_string(count++);
        logger->fatal(__FILE__, __LINE__, tmp.c_str());
        cursize += tmp.size();
    }*/
}

int main()
{
    //传给path函数的string，./后面跟上路径就是在当前目录创建，直接写/路径，就是把路径创建和root同目录下
    //std::string pn = "./lf/a/b/c.cc";
    //Logs::LogUtil::File::createDirectory(Logs::LogUtil::File::path(pn));

    //Logs::LogMsg msg(Logs::LogLevel::value::Info, 47, "main.c", "root", "格式化功能测试");
    //Logs::Formatter fmt;
    //Logs::Formatter fmt("abc%%abc%g%g%g[%d{%H:%M:%S}] %m%n%g");
    //std::string str = fmt.format(msg);
    //std::cout << str << std::endl;

    /*Logs::LogSink::ptr stdout_lsp = Logs::SinkFactory::create<Logs::StdoutSink>();
    Logs::LogSink::ptr file_lsp = Logs::SinkFactory::create<Logs::FileSink>("./logfile/file.log");
    stdout_lsp->log(str.c_str(), str.size());
    file_lsp->log(str.c_str(), str.size());*/
    /*Logs::LogSink::ptr roll_lsp = Logs::SinkFactory::create<Logs::RollBySizeSink>("./logfile/roll-", 1024 * 1024);
    size_t cursize = 0;
    size_t count = 0;
    while(cursize < 1024 * 1024 * 10)//小于10MB就持续写入
    {
        std::string tmp = std::to_string(count++) + " " + str;
        roll_lsp->log(tmp.c_str(), tmp.size());
        cursize += tmp.size();
    }*/
    /*Logs::LogSink::ptr time_lsp = Logs::SinkFactory::create<RollByTimeSink>("./logfile/roll-", TimeGap::GAP_SECOND);
    time_t old = Logs::LogUtil::Date::now();
    while(Logs::LogUtil::Date::now() < old + 5)
    {
        time_lsp->log(str.c_str(), str.size());
        usleep(1000);
    }*/


    /*std::unique_ptr<Logs::Builder> builder(new Logs::LocalLoggerBuilder());
    builder->buildLoggerType(Logs::Logger::Type::LOGGER_SYNC);
    builder->buildLoggerName("sync_logger");
    builder->buildLoggerLevel(Logs::LogLevel::value::Warn);
    builder->buildFormatter("%m%n");
    builder->buildSink<Logs::FileSink>("./logfile/test.log");
    builder->buildSink<Logs::StdoutSink>();
    Logs::Logger::ptr logger = builder->build();
    logger->debug(__FILE__, __LINE__, "%s", "测试日志d");
    logger->info(__FILE__, __LINE__, "%s", "测试日志i");
    logger->warn(__FILE__, __LINE__, "%s", "测试日志w");
    logger->error(__FILE__, __LINE__, "%s", "测试日志e");
    logger->fatal(__FILE__, __LINE__, "%s", "测试日志f");

    size_t cursize = 0, count = 0;
    std::string str = "测试日志-";
    while(cursize < 1024 * 1024 * 10)//小于10MB就持续写入
    {
        std::string tmp = std::to_string(count++) + str;
        logger->fatal(__FILE__, __LINE__, tmp.c_str());
        cursize += tmp.size();
    }*/


    //异步日志器测试
    /*std::unique_ptr<Logs::Builder> builder(new Logs::LocalLoggerBuilder());
    builder->buildLoggerType(Logs::Logger::Type::LOGGER_ASYNC);
    builder->buildLoggerName("async_logger");
    builder->buildLoggerLevel(Logs::LogLevel::value::Warn);
    builder->buildFormatter("[%c]%m%n");
    builder->buildSink<Logs::FileSink>("./logfile/async.log");
    builder->buildSink<Logs::StdoutSink>();
    Logs::Logger::ptr logger = builder->build();

    logger->debug(__FILE__, __LINE__, "%s", "测试日志d");
    logger->info(__FILE__, __LINE__, "%s", "测试日志i");
    logger->warn(__FILE__, __LINE__, "%s", "测试日志w");
    logger->error(__FILE__, __LINE__, "%s", "测试日志e");
    logger->fatal(__FILE__, __LINE__, "%s", "测试日志f");

    size_t count = 0;
    std::string str = "测试日志-";
    while(count < 500000)
    {
        std::string tmp = std::to_string(count++) + str;
        logger->fatal(__FILE__, __LINE__, tmp.c_str());
    }*/


    /*std::unique_ptr<Logs::Builder> builder(new Logs::GlobalLoggerBuilder());
    builder->buildLoggerType(Logs::Logger::Type::LOGGER_ASYNC);
    builder->buildLoggerName("async_logger");
    builder->buildLoggerLevel(Logs::LogLevel::value::Warn);
    builder->buildFormatter("[%c][%f:%l]%m%n");
    builder->buildSink<Logs::FileSink>("./logfile/file.log");
    builder->buildSink<Logs::StdoutSink>();
    builder->build();*/
    test_log();


    return 0;
}