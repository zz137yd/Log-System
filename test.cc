#include "logs.h"

void test_log()
{
    //Time
    /*Logs::Logger::ptr logger = Logs::LoggerManager::getInstance().getLogger("async_logger");
    logger->debug(__FILE__, __LINE__, "%s", "test logd");
    logger->info(__FILE__, __LINE__, "%s", "test logi");
    logger->warn(__FILE__, __LINE__, "%s", "test logw");
    logger->error(__FILE__, __LINE__, "%s", "test loge");
    logger->fatal(__FILE__, __LINE__, "%s", "test logf");
    size_t count = 0;
    std::string str = "test log-";
    while(count < 470000)
    {
        std::string tmp = str + std::to_string(count++);
        logger->fatal(__FILE__, __LINE__, tmp.c_str());
    }*/
    /*DEBUG("%s", "test log");
    INFO("%s", "test log");
    WARN("%s", "test log");
    ERROR("%s", "test log");
    FATAL("%s", "test log");
    size_t count = 0;
    while(count < 470000)
    {
        FATAL("test log-%d", count++);
    }*/


    //Size
    /*size_t cursize = 0, count = 0;
    std::string str = "test log-";
    while(cursize < 1024 * 1024 * 10)//Keep writing if it's less than 10MB
    {
        std::string tmp = str + std::to_string(count++);
        logger->fatal(__FILE__, __LINE__, tmp.c_str());
        cursize += tmp.size();
    }*/
}

int main()
{
    //In the string passed to the path function, ./ followed by the path means it will be created in the current directory
    //Writing /path directly will create the path in the same directory as root

    //std::string pn = "./lf/a/b/c.cc";
    //Logs::LogUtil::File::createDirectory(Logs::LogUtil::File::path(pn));

    //Logs::LogMsg msg(Logs::LogLevel::value::Info, 47, "main.c", "root", "format function test");
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
    while(cursize < 1024 * 1024 * 10)//Keep writing if it's less than 10MB
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
    logger->debug(__FILE__, __LINE__, "%s", "test logd");
    logger->info(__FILE__, __LINE__, "%s", "test logi");
    logger->warn(__FILE__, __LINE__, "%s", "test logw");
    logger->error(__FILE__, __LINE__, "%s", "test loge");
    logger->fatal(__FILE__, __LINE__, "%s", "test logf");

    size_t cursize = 0, count = 0;
    std::string str = "test log-";
    while(cursize < 1024 * 1024 * 10)//Keep writing if it's less than 10MB
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

    logger->debug(__FILE__, __LINE__, "%s", "test logd");
    logger->info(__FILE__, __LINE__, "%s", "test logi");
    logger->warn(__FILE__, __LINE__, "%s", "test logw");
    logger->error(__FILE__, __LINE__, "%s", "test loge");
    logger->fatal(__FILE__, __LINE__, "%s", "test logf");

    size_t count = 0;
    std::string str = "test log-";
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