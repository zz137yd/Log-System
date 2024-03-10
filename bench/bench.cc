#ifndef __M_BENCH_H__
#define __M_BENCH_H__

#include "../logs.h"
#include <chrono>

static int n = 0;

void bench(const std::string& logger_name, size_t thread_num, size_t msg_count, size_t msg_len)
{
    std::cout << "\nStart testn"; 
    //1、Get logger
    Logs::Logger::ptr logger = Logs::getLogger(logger_name);
    if(logger.get() == nullptr) return ;
    //2、Organize log messages
    std::string msg(msg_len - 1, '1');//One less byte is to add a newline at the end
    //3、Create specified number of threads
    std::vector<std::thread> threads;
    std::vector<double> cost_time(thread_num);//Calculate the time each thread spends working
    size_t msg_count_per_thread = msg_count / thread_num;//The number of logs to be output by each thread
    std::cout << "\nNumber of worker threads: " << thread_num << std::endl;
    std::cout << "Number of output logs: " << msg_count << std::endl;
    std::cout << "Total output log volume: " << (msg_len * msg_count) / 1024  << "KB" << std::endl;
    std::cout << std::endl;
    for(int i = 0; i < thread_num; ++i)
    {
        //Compared with push_back, emplace_back constructs this object directly at the end of the container instead of constructing a temporary object and then inserting it
        threads.emplace_back([&, i](){
            //4、Start timing inside the thread
            auto start = std::chrono::high_resolution_clock::now();
            //5、Start for loop to write log
            for(int j = 0; j < msg_count_per_thread; ++j)
            {
                logger->FaTal("%s", msg.c_str());
            }
            //6、End timing inside the thread
            auto end = std::chrono::high_resolution_clock::now();
            //auto cost = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
            std::chrono::duration<double> cost = end - start;
            cost_time[i] = cost.count();
            auto avg = msg_count_per_thread / cost_time[i];
            std::cout << "Thread" << i << "takes time: " << cost.count() << "s, " << " average: " << (size_t)avg << "/s\n";
        });
    }
    for(auto& it : threads)
    {
        it.join();
    }
    //7、Calculate the total time taken (because threads are processed concurrently, the highest time consuming is the total time)
    double max_cost = 0;
    for(int i = 0; i < thread_num; ++i)
    {
        max_cost = max_cost < cost_time[i] ? cost_time[i] : max_cost;
    }
    size_t msg_per_sec = msg_count / max_cost;
    size_t size_per_sec = (msg_count * msg_len) / (max_cost * 1024 * 1024);//in MB
    //8、Print output
    std::cout << "\nTotal time spent: " << max_cost << "s\n";
    std::cout << "Number of output logs per second: " << msg_per_sec << "\n";
    std::cout << "Output log size per second: " << size_per_sec << "MB\n";
    std::cout << "————————————————————————————————————" << std::endl;
}

void sync_bench(size_t threadcount, size_t msgcount, size_t msglen)
{
    const std::string logger_name = "sync_log -- " + std::to_string(n);
    INFO("************************************************");
    INFO("Synchronous logger test: %d threads, %d messages", threadcount, msgcount);

    //Also Can be written as Logs::GlobalLoggerBuilder::ptr builder...
    std::unique_ptr<Logs::GlobalLoggerBuilder> builder(new Logs::GlobalLoggerBuilder());
    builder->buildLoggerType(Logs::Logger::Type::LOGGER_SYNC);
    builder->buildLoggerName(logger_name);
    builder->buildFormatter("%m");
    std::string path = "./logs/sync" + std::to_string(n++) + ".log";
    builder->buildSink<Logs::FileSink>(path);
    builder->build();
    bench(logger_name, threadcount, msgcount, msglen);
    INFO("************************************************");
}

void async_bench(size_t threadcount, size_t msgcount, size_t msglen)
{
    std::string logger_name = "async_log -- " + std::to_string(n);
    INFO("************************************************");
    INFO("Synchronous logger test: %d threads, %d messages", threadcount, msgcount);

    //The asynchronous logger writes to the memory first, and then writes to the disk from the memory
    //Also Can be written as Logs::GlobalLoggerBuilder::ptr builder...
    std::unique_ptr<Logs::GlobalLoggerBuilder> builder(new Logs::GlobalLoggerBuilder());
    builder->buildLoggerType(Logs::Logger::Type::LOGGER_ASYNC);
    builder->buildLoggerName(logger_name);
    builder->buildFormatter("%m");
    //In order to test performance, the actual landing time is excluded
    std::string path = "./logs/sync" + std::to_string(n++) + ".log";
    builder->buildSink<Logs::FileSink>(path);
    builder->build();
    bench(logger_name, threadcount, msgcount, msglen);
    INFO("************************************************");
}

int main()
{
    //Synchronous
    sync_bench(1, 7000000, 100);
    //sync_bench(4, 3000000, 100);

    //Asynchronous
    //async_bench(1, 3000000, 100);
    //async_bench(4, 200000, 100);
    return 0;
}

#endif