#ifndef __M_SINK_H__
#define __M_SINK_H__

/*Implementation of log sink module
    1、Abstract base class for log sink
    2、Derived classes (derived according to different landing directions)
    3、Use factory pattern to separate creation and presentation
*/

#include "util.hpp"
#include <fstream>
#include <sstream>
#include <memory>
#include <cassert>

static void R_Create(const std::string& pathname, std::ofstream& ofs)
{
    // 1、Create the log files directory
    Logs::LogUtil::File::createDirectory(Logs::LogUtil::File::path(pathname));
    // 2、Create and open log file
    //The second parameter indicates the opening mode, binary mode, the default is writing
    //Now we need to append, so there is content after |
    ofs.open(pathname, std::ios::binary | std::ios::app); 
    assert(ofs.is_open());//Ensure successful opening
}
 
namespace Logs
{
    class LogSink
    {
    public:
        //Each module accesses each other through abstraction and pointers, so smart pointer is used
        using ptr = std::shared_ptr<LogSink>;
        LogSink() {}
        virtual ~LogSink() {}
        virtual void log(const char* data, size_t len) = 0;
    };


    //Direction: standard output
    class StdoutSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<StdoutSink>;

        StdoutSink() = default;
        void log(const char* data, size_t len)
        {
            //Can't use stream input directly
            //Use the overloaded function write in the library to write to the standard output file
            std::cout.write(data, len);//Write contents of length len from data
        }
    };


    //Direction: specified file
    class FileSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<FileSink>;

        //Open the file during construction and manage the operation handle
        FileSink(const std::string& filename):_filename(filename)
        {
            R_Create(_filename, _ofs);
        }

        const std::string& file() {return _filename; }

        //Write log messages to file
        void log(const char* data, size_t len)
        {
            _ofs.write(data, len);
            //Check whether the current handle is normal
            //That is, whether there is any abnormality after writing above, and exit directly if so
            if (_ofs.good() == false) std::cout << "Log output file failed! \n";
        }
    private:
        std::string _filename;
        std::ofstream _ofs;//Write to log via handle
    };


    //Direction: rolling file
    class RollBySizeSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<RollBySizeSink>;

        //Open the file when it's constructed and manages the operation handle
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
            if (_ofs.good() == false) std::cout << "Space-differentiated log file write failed! \n";
            _cur_fsize += len;
        }
    private:
        //There is no stipulation on the maximum file size, so the file size will vary
        //Check before each write
        //If it is larger than the specified maximum size after the last write, close the file, reopen one, and let _ofs point to the new file
        void InitLogFile()
        {
            if (_ofs.is_open() == false || _cur_fsize >= _max_fsize)
            {
                _ofs.close();//Close the current file first, then open the new file
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
            //Get the system time and use the time to construct the file name extension
            time_t t = time(NULL);
            struct tm lt;
            localtime_r(&t, &lt);//Convert timestamp to time structure
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
        size_t _name_count;//When writing content is small, prevent a large number of files from being generated in an instant
        //Base file name + extended file name (generated in time) form the actual file name of the current output
        std::string _basename;//Example: .//log/base-20240120.log
        std::ofstream _ofs;
        size_t _max_fsize;//maximum file size
        size_t _cur_fsize;//The size of the data written to the current file
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
                std::cout << "Time-differentiated log file writing failed! \n";
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
            //Get the system time and use the time to construct the file name extension
            time_t t = Logs::LogUtil::Date::now();
            struct tm lt;
            localtime_r(&t, &lt);//Convert timestamp to time structure
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

    //Even if new directions are added in the future, the factory can produce them
    //Use templates to comply with the opening and closing principle
    //Derived classes in different landing directions pass different numbers of parameters, so indefinite parameters are used
    class SinkFactory
    {
    public:
        //Written as a template function, when called from outside, it will be written as...create<>
        //If the template is placed above the class, it will be written as...SinkFactory<>
        template <typename SinkType, typename... Args>
        static LogSink::ptr create(Args &&...args)
        {
            return std::make_shared<SinkType>(std::forward<Args>(args)...);
        }
    };
}

#endif