/*Implement asynchronous logger*/

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
        ASYNC_SAFE,//Blocked when full
        ASYNC_UNSAFE//Unlimited expansion for testing
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
            _stop = true;//Corresponds to worker_loop
            _pop_cond.notify_all();
            _thread.join();//Wait for the worker thread to exit and then recycle
        }

        void push(const std::string &msg)
        {
            if (_stop) return;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                //Directly implemented as blocking waiting, which is the original AsyncType::ASYNC_SAFE
                _push_cond.wait(lock, [&]{ return _tasks_push.writeAbleSize() >= msg.size(); });
                _tasks_push.push(msg.c_str(), msg.size());
            }
            _pop_cond.notify_all();
        }
    private:
        //threadRoutine function
        //After the thread is awakened, it will execute this function
        //Process the data in the consumption buffer, initialize the buffer after processing, and exchange the buffer
        void worker_loop()
        {
            while(1)
            {
                {//{} is to set a life cycle so that the added lock will be automatically unlocked after the exchange.
                    // 1、Determine whether there is data in the production buffer, exchange if there is, or block if not
                    std::unique_lock<std::mutex> lock(_mutex);
                    if (_stop && _tasks_push.empty()) { return; }//Prevent the production buffer from exiting without processing data
                    //This means that there is still data in the production buffer or the whole is about to stop working
                    _pop_cond.wait(lock, [&]{ return !_tasks_push.empty() || _stop; });
                    _tasks_push.swap(_tasks_pop);
                }
                //2、Wake up producers
                //The code has been implemented in blocking mode. Only when blocked can threads need to be awakened
                _push_cond.notify_all();
                //3、After waking up, perform data processing on the consumption buffer
                _callBack(_tasks_pop);
                //4、Initialize consumption buffer
                _tasks_pop.reset();
            }
            return ;
        }
    private:
        std::atomic<bool> _stop;//Used to stop logger
        std::mutex _mutex;
        std::thread _thread;//Async worker worker thread
        Functor _callBack;//Callback function for buffer data processing
        AsyncType _looper_type;
        std::condition_variable _push_cond;//Producer condition variable
        std::condition_variable _pop_cond;//Consumer condition variable
        Buffer _tasks_push;//Production buffer
        Buffer _tasks_pop;//Consumption buffer
    };
}

#endif