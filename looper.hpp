/*实现异步工作器*/

#ifndef __M_LOOP_H__
#define __M_LOOP_H__

#include "buffer.hpp"
#include "util.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <atomic>

namespace Logs
{
    //using Functor = std::function<void(Buffer &)>;

    enum class AsyncType
    {
        ASYNC_SAFE,//满了就阻塞
        ASYNC_UNSAFE//无限扩容, 用于测试 
    };

    class AsyncLooper
    {
    public:
        using Functor = std::function<void(Buffer& buffer)>;
        using ptr = std::shared_ptr<AsyncLooper>;

        AsyncLooper(const Functor &cb)//, AsyncType loop_type = AsyncType::ASYNC_SAFE
            : _stop(false), _thread(std::thread(&AsyncLooper::worker_loop, this)), _callBack(cb)//, _looper_type(loop_type)
        {}

        ~AsyncLooper() {stop(); };

        void stop()
        {
            _stop = true;//对应下面的worker_loop
            _pop_cond.notify_all();
            _thread.join();//等待工作线程退出然后回收
        }

        void push(const std::string &msg)
        {
            if (_stop) return;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                //直接就实现为阻塞等待, 也就是原来的AsyncType::ASYNC_SAFE
                _push_cond.wait(lock, [&]{ return _tasks_push.writeAbleSize() >= msg.size(); });
                _tasks_push.push(msg.c_str(), msg.size());
            }
            _pop_cond.notify_all();
        }
    private:
        //线程入口函数
        //线程被唤醒后就自己执行这个函数
        void worker_loop()//处理消费缓冲区内的数据, 处理完后初始化缓冲区, 交换缓冲区
        {
            while(1)
            {
                {//这种大括号的作用是设定一个生命周期, 这样加的锁在交换后就会自动解锁
                    // 1、判断生产缓冲区有没有数据, 有则交换, 无则阻塞
                    std::unique_lock<std::mutex> lock(_mutex);
                    if (_stop && _tasks_push.empty()) { return; }//防止生产缓冲区还有数据没有处理就退出了
                    _pop_cond.wait(lock, [&]{ return !_tasks_push.empty() || _stop; });//意思就是生产缓冲区还有数据或者整体要停止工作了
                    _tasks_push.swap(_tasks_pop);
                }
                //2、唤醒生产者
                _push_cond.notify_all();//代码已经实现为阻塞模式, 只有阻塞才有线程需要被唤醒
                //3、被唤醒后, 对消费缓冲区进行数据处理
                _callBack(_tasks_pop);
                //4、初始化消费缓冲区
                _tasks_pop.reset();
            }
            return ;
        }
    private:
        std::atomic<bool> _stop;//用来停止工作器
        std::mutex _mutex;
        std::thread _thread;//异步工作器对应的工作线程
        Functor _callBack;//对缓冲区数据处理用的回调函数
        AsyncType _looper_type;
        std::condition_variable _push_cond;//生产者条件变量
        std::condition_variable _pop_cond;//_消费者条件变量
        Buffer _tasks_push;//生产缓冲区
        Buffer _tasks_pop;//消费缓冲区
    };
}

#endif