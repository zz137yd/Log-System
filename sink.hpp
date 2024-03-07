#ifndef __M_SINK_H__
#define __M_SINK_H__

/*日志落地模块的实现
    1、抽象落地基类
    2、派生子类（根据不同的落地方向进行派生）
    3、使用工厂模式进行创建与表示的分离
*/

#include "util.hpp"
#include <fstream>
#include <sstream>
#include <memory>
#include <cassert>

static void R_Create(const std::string& pathname, std::ofstream& ofs)
{
    // 1、创建日志文件所在的目录
    Logs::LogUtil::File::createDirectory(Logs::LogUtil::File::path(pathname));
    // 2、创建并打开日志文件
    ofs.open(pathname, std::ios::binary | std::ios::app); //第二个参数是打开方式, 二进制方式; 默认为写, 这里需要追加, 所以有|后的内容
    assert(ofs.is_open());//保证打开成功
}
 
namespace Logs
{
    class LogSink
    {
    public:
        using ptr = std::shared_ptr<LogSink>;//各个模块之间是通过抽象, 通过指针来互相访问的, 所以需要智能指针
        LogSink() {}
        virtual ~LogSink() {}
        virtual void log(const char* data, size_t len) = 0;
    };


    //落地方向: 标准输出
    class StdoutSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<StdoutSink>;

        StdoutSink() = default;
        void log(const char* data, size_t len)
        {
            //不能直接用流输入, 用库里带的重载函数write写到标准输出文件中
            std::cout.write(data, len);//从data位置写len长度的数据
        }
    };


    //落地方向: 指定文件
    class FileSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<FileSink>;

        //构造时就打开文件, 将操作句柄管理起来
        FileSink(const std::string& filename):_filename(filename)
        {
            R_Create(_filename, _ofs);
        }

        const std::string& file() {return _filename; }

        //将日志消息写入到文件
        void log(const char* data, size_t len)
        {
            _ofs.write(data, len);
            //查看当前句柄是否正常, 也就是上面write后是否有异常情况, 有就直接退出
            if (_ofs.good() == false) std::cout << "日志输出文件失败！\n";
        }
    private:
        std::string _filename;
        std::ofstream _ofs;//通过句柄来写入日志
    };


    //落地方向: 滚动文件
    class RollBySizeSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<RollBySizeSink>;

        //构造时就打开文件, 将操作句柄管理起来
        RollBySizeSink(const std::string& basename, size_t max_size)
            : _basename(basename), _max_fsize(max_size), _cur_fsize(0), _name_count(0)
        {
            //LogUtil::File::createDirectory(LogUtil::File::path(basename));
            std::string pathname = createFilename();
            R_Create(pathname, _ofs);
        }

        void log(const char* data, size_t len)
        {
            InitLogFile();
            _ofs.write(data, len);
            if (_ofs.good() == false) std::cout << "以空间区分的日志文件写入失败！\n";
            _cur_fsize += len;
        }
    private:
        //这里并没有硬性规定文件最大有多少, 所以文件大小会出现不一样
        //每次写入前都检查一下, 如果上次写完后大于自己规定的最大大小了, 那就关闭这个文件, 重开一个, 让_ofs指向新文件
        void InitLogFile()
        {
            if (_ofs.is_open() == false || _cur_fsize >= _max_fsize)
            {
                _ofs.close();//先关闭当前文件, 再打开新文件
                std::string pathname = createFilename();
                _ofs.open(pathname, std::ios::binary | std::ios::app);
                assert(_ofs.is_open());
                _cur_fsize = 0;
                return ;
            }
            return ;
        }

        std::string createFilename()
        {
            //获取系统时间, 以时间来构造文件名扩展名
            time_t t = time(NULL);
            struct tm lt;
            localtime_r(&t, &lt);//将时间戳转换为时间结构
            std::stringstream ss;
            ss << _basename;
            ss << lt.tm_year + 1900;
            ss << lt.tm_mon + 1;
            ss << lt.tm_mday;
            ss << lt.tm_hour;
            ss << lt.tm_min;
            ss << lt.tm_sec;
            ss << "--";
            ss << _name_count++;
            ss << ".logsize";
            return ss.str();
        }
    private:
        size_t _name_count;//当写入内容小时, 防止一瞬间产生大量文件
        //基础文件名 + 扩展文件名（以时间生成）组成当前输出的实际文件名
        std::string _basename;//比如.//log/base-20240120.log
        std::ofstream _ofs;
        size_t _max_fsize;//文件最大大小
        size_t _cur_fsize;//当前文件已写入数据的大小
    };

    enum class TimeGap
    {
        GAP_SECOND,
        GAP_MINUTE,
        GAP_HOUR,
        GAP_DAY,
    };

    class RollByTimeSink : public LogSink
    {
    public:
        RollByTimeSink(const std::string& basename, TimeGap gap_type)
            : _basename(basename), _name_count(0)
        {
            switch (gap_type)
            {
            case TimeGap::GAP_SECOND:
                _gap_size = 1;
                break;
            case TimeGap::GAP_MINUTE:
                _gap_size = 60;
                break;
            case TimeGap::GAP_HOUR:
                _gap_size = 3600;
                break;
            case TimeGap::GAP_DAY:
                _gap_size = 3600 * 24;
                break;
            }
            std::string filename = createFilename();
            R_Create(filename, _ofs);
            _cur_gap = Logs::LogUtil::Date::now();
        }

        void log(const char *data, size_t len)
        {
            InitLogFile();
            _ofs.write(data, len);
            if (_ofs.good() == false)
                std::cout << "以时间区分的日志文件写入失败！\n";
        }

    private:
        void InitLogFile()
        {
            time_t cur = Logs::LogUtil::Date::now();
            if (cur >= _cur_gap + _gap_size)
            {
                _ofs.close();
                std::string filename = createFilename();
                _ofs.open(filename, std::ios::binary | std::ios::app);
                assert(_ofs.is_open());
                _cur_gap = cur;
            }
        }

        std::string createFilename()
        {
            //获取系统时间, 以时间来构造文件名扩展名
            time_t t = Logs::LogUtil::Date::now();
            struct tm lt;
            localtime_r(&t, &lt);//将时间戳转换为时间结构
            std::stringstream sst;
            sst << _basename;
            sst << lt.tm_year + 1900;
            sst << lt.tm_mon + 1;
            sst << lt.tm_mday;
            sst << lt.tm_hour;
            sst << lt.tm_min;
            sst << lt.tm_sec;
            sst << "--";
            sst << _name_count++;
            sst << ".logtime";
            return sst.str();
        }

    private:
        std::string _basename;
        std::ofstream _ofs;
        size_t _cur_gap;
        size_t _gap_size;
        size_t _name_count;
    };

    //即使后续要新增落地方向, 工厂也能生产出来
    //利用模版来符合开闭原则
    //不同落地方向的派生类传的参数数量不同, 所以用不定参数
    class SinkFactory
    {
    public:
        //写成模版函数, 如果是模版放在class上面, 外面调用时就得写成...SinkFactory<>, 模版函数则是...create<>
        template <typename SinkType, typename... Args>
        static LogSink::ptr create(Args &&...args)
        {
            return std::make_shared<SinkType>(std::forward<Args>(args)...);
        }
    };
}

#endif