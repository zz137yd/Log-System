#ifndef __M_BENCH_H__
#define __M_BENCH_H__

#include "../logs.h"
#include <chrono>

static int n = 0;

void bench(const std::string& logger_name, size_t thread_num, size_t msg_count, size_t msg_len)
{
    std::cout << "\n开始测试\n"; 
    //1、获取日志器
    Logs::Logger::ptr logger = Logs::getLogger(logger_name);
    if(logger.get() == nullptr) return ;
    //2、组织日志消息
    std::string msg(msg_len - 1, '1');//少一个字节是为了给末尾添加换行
    //3、创建指定数量线程
    std::vector<std::thread> threads;
    std::vector<double> cost_time(thread_num);//计算每个线程工作时花费的时间
    size_t msg_count_per_thread = msg_count / thread_num;//每个线程要输出的日志数量
    std::cout << "\n工作线程数量: " << thread_num << std::endl;
    std::cout << "输出日志数量: " << msg_count << std::endl;
    std::cout << "输出日志总量: " << (msg_len * msg_count) / 1024  << "KB" << std::endl;
    std::cout << std::endl;
    for(int i = 0; i < thread_num; ++i)
    {
        //相比于push_back, emplace_back直接在容器尾部构造这个对象, 而不是构造一个临时对象再插入
        threads.emplace_back([&, i](){
            //4、线程函数内部开始计时
            auto start = std::chrono::high_resolution_clock::now();
            //5、开始循环写日志
            for(int j = 0; j < msg_count_per_thread; ++j)
            {
                logger->FaTal("%s", msg.c_str());
            }
            //6、线程函数内部结束计时
            auto end = std::chrono::high_resolution_clock::now();
            //auto cost = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
            std::chrono::duration<double> cost = end - start;
            cost_time[i] = cost.count();
            auto avg = msg_count_per_thread / cost_time[i];
            std::cout << "线程" << i << "号耗时: " << cost.count() << "s, " << " 平均: " << (size_t)avg << "/s\n";
        });
    }
    for(auto& it : threads)
    {
        it.join();
    }
    //7、计算总耗时（因为线程并发处理, 所以耗时最高的就是总时间）
    double max_cost = 0;
    for(int i = 0; i < thread_num; ++i)
    {
        max_cost = max_cost < cost_time[i] ? cost_time[i] : max_cost;
    }
    size_t msg_per_sec = msg_count / max_cost;
    size_t size_per_sec = (msg_count * msg_len) / (max_cost * 1024 * 1024);//以MB为单位
    //8、进行输出打印
    std::cout << "\n总耗时: " << max_cost << "s\n";
    std::cout << "每秒输出日志数量: " << msg_per_sec << "条\n";
    std::cout << "每秒输出日志大小: " << size_per_sec << "MB\n";
    std::cout << "————————————————————————————————————" << std::endl;
}

void sync_bench(size_t threadcount, size_t msgcount, size_t msglen)
{
    const std::string logger_name = "sync_log -- " + std::to_string(n);
    INFO("************************************************");
    INFO("同步日志测试: %d threads, %d messages", threadcount, msgcount);

    //也可以写成Logs::GlobalLoggerBuilder::ptr builder...
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
    INFO("同步日志测试: %d threads, %d messages", threadcount, msgcount);

    //异步是先写到内存, 再从内存到磁盘
    //也可以写成Logs::GlobalLoggerBuilder::ptr builder...
    std::unique_ptr<Logs::GlobalLoggerBuilder> builder(new Logs::GlobalLoggerBuilder());
    builder->buildLoggerType(Logs::Logger::Type::LOGGER_ASYNC);
    builder->buildLoggerName(logger_name);
    builder->buildFormatter("%m");
    //为了测试性能, 将实际落地时间排除在外
    std::string path = "./logs/sync" + std::to_string(n++) + ".log";
    builder->buildSink<Logs::FileSink>(path);
    builder->build();
    bench(logger_name, threadcount, msgcount, msglen);
    INFO("************************************************");
}

int main()
{
    //同步
    sync_bench(1, 7000000, 100);
    //sync_bench(4, 3000000, 100);

    //异步
    //async_bench(1, 3000000, 100);
    //async_bench(4, 200000, 100);
    return 0;
}

#endif