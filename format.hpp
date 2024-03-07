#ifndef __M_FMT_H__
#define __M_FMT_H__

#include "message.hpp"
#include <vector>
#include <cassert>
#include <sstream>
#include <tuple>//元组

namespace Logs
{
    class FormatItem
    {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual ~FormatItem() {}
        virtual void format(std::ostream& out, const LogMsg& msg) = 0;
    };

    class MsgFormatItem : public FormatItem
    {
    public:
        MsgFormatItem(const std::string& str = ""){}
        void format(std::ostream& out, const LogMsg& msg) override
        {
            out << msg._payload;
        }
    };

    class LevelFormatItem : public FormatItem
    {
    public:
        LevelFormatItem(const std::string& str = ""){}
        void format(std::ostream& out, const LogMsg& msg) override
        {
            out << LogLevel::toString(msg._level);
        }
    };
    
    class TimeFormatItem : public FormatItem//时间有子格式
    {
    public:
        TimeFormatItem(const std::string& fmt = "%H:%M:%S"):_time_fmt(fmt)
        {
            if(fmt.empty()) _time_fmt = "%H:%M:%S";
        }

        void format(std::ostream& out, const LogMsg& msg) override
        {
            struct tm t;
            localtime_r(&msg._ctime, &t);//从第一个参数中获取当前系统时间, 放到t中, 返回tm结构体的指针
            char tmp[128] = {0};
            //按照格式来格式化t中的时间, 放到tmp中, 数字表示复制到tmp中的最大字符数, 必须大于格式字符数
            strftime(tmp, 127, _time_fmt.c_str(), &t);
            out << tmp;//放到输出流中
        }
    private:
        std::string _time_fmt;//默认是%H:%M:%S
    };

    class FileFormatItem : public FormatItem
    {
    public:
        FileFormatItem(const std::string& str = ""){}
        void format(std::ostream& out, const LogMsg& msg) override
        {
            out << msg._file;
        }
    };

    class LineFormatItem : public FormatItem
    {
    public:
        LineFormatItem(const std::string& str = ""){}
        void format(std::ostream& out, const LogMsg& msg) override
        {
            out << msg._line;
        }
    };

    class ThreadFormatItem : public FormatItem
    {
    public:
        ThreadFormatItem(const std::string& str = ""){}
        void format(std::ostream& out, const LogMsg& msg) override
        {
            out << msg._tid;
        }
    };

    class NameFormatItem : public FormatItem
    {
    public:
        NameFormatItem(const std::string& str = ""){}
        void format(std::ostream& out, const LogMsg& msg) override
        {
            out << msg._name;
        }
    };

    class TabFormatItem : public FormatItem
    {
    public:
        TabFormatItem(const std::string& str = ""){}
        void format(std::ostream& out, const LogMsg& msg) override
        {
            out << "\t";
        }
    };

    class NLineFormatItem : public FormatItem
    {
    public:
        NLineFormatItem(const std::string& str = ""){}
        void format(std::ostream& out, const LogMsg& msg) override
        {
            out << "\n";
        }
    };

    class OtherFormatItem : public FormatItem
    {
    public:
        OtherFormatItem(const std::string& str = ""):_str(str) {}
        void format(std::ostream& out, const LogMsg& msg) override
        {
            out << _str;
        }
    private:
        std::string _str; 
    };

    /*
        %d 表示日期, 包含子格式 {%H:%M:%S}
        %t 表示线程ID
        %c 表示日志器名称
        %f 表示源码文件名
        %l 表示源码行号
        %p 表示日志级别
        %T 表示制表符缩进
        %m 表示主体消息
        %n 表示换行
    */

    class Formatter
    {
    public:
        using ptr = std::shared_ptr<Formatter>;

        Formatter(const std::string& pattern = "[%d{%H:%M:%S}][%t][%p][%c][%f:%l] %m%n") : _pattern(pattern)
        {
            assert(parsePattern());
        }

        const std::string pattern() {return _pattern; }

        //重载, 对消息进行格式化后将字符串放到IO流中
        std::ostream& format(std::ostream& out, const LogMsg& msg)
        {
            //按照解析的顺序逐个从msg里取出对应的信息
            for(auto& item : _items)
            {
                //每个子类都重载了format
                item->format(out, msg);
            }
            return out;
        }

        //对消息进行格式化, 返回格式化后的字符串
        std::string format(const LogMsg& msg)
        {
            std::stringstream ss;
            for(auto& it : _items) it->format(ss, msg);
            return ss.str();
        }
    private:
        //对格式化规则字符串进行解析并添加到数组中
        bool parsePattern()
        {
            //1、对格式化规则字符串进行解析
            std::vector<std::tuple<std::string, std::string, int>> arr;//元组不可以修改
            //std::vector<std::pair<std::string, std::string>> fmt_order;//pair对应的就是字典的用法

            size_t pos = 0;
            bool sub_format_error = false;//用来判断是否正确拿到子格式
            std::string key;//存放%后的格式化字符
            std::string val;//存放格式化字符后{}中的子格式字符串
            std::string string_row;//存放非格式化字符

            //默认字符串形式: [%d{%H:%M:%S}][%t][%p][%c][%f:%l] %m%n
            while(pos < _pattern.size())
            {
                if(_pattern[pos] != '%')
                {
                    //这里有++, 循环上去后有可能就越界退出while了
                    //此时string_row已经有了内容, 所以while结束后还得处理一下它
                    string_row.append(1, _pattern[pos++]);//string& append (size_t n, char c), 填写n个c
                    continue;
                }
                //到这里pos指向第一个%

                //有可能出现%%, 有可能出现[%d%%%F%%F%%%%d]这样的
                //这里的规则就是允许一个[]内有多个格式化字符, 但必须都是%z这样的, z前有多个%都看作一个%
                while(pos + 1 < _pattern.size() && _pattern[pos + 1] == '%')
                    ++pos;
                //此时pos指向%, %后必然有一个字符, 但不知道是不是规定的格式化字符 

                //先处理非格式化字符
                if(string_row.empty() == false)
                {
                    //利用元组不可修改的特性, 就固定好了非格式化字符的val就是变为"", 之后调用OtherFormatItem
                    arr.push_back(std::make_tuple(string_row, "", 0));
                    string_row.clear();
                }

                ++pos;//pos指向%后字符的起始位置

                if (pos < _pattern.size() && isalpha(_pattern[pos]))
                {
                    key = _pattern[pos];//保存格式化字符
                }
                else//要是没有格式化字符, 且上面也判断不是%, 就出错了
                {
                    std::cout << "从此处开始 " << &_pattern[pos - 1] << " 格式错误！\n";
                    return false;
                }
                //也就是说, 上面只判断%, 至于%后的字符怎么样，交给这里的ifelse来处理

                ++pos;//这里看看有没有子格式, 例子: %d{...}, 此时pos在{

                //这里也要判断是否越界
                if (pos < _pattern.size() && _pattern[pos] == '{')
                {
                    sub_format_error = true;
                    pos += 1;//pos指向{后, 即子格式第一个字符位置
                    while (pos < _pattern.size())
                    {
                        if (_pattern[pos] == '}')//要么是{}, 要么是{...}, 两者形式都对
                        {
                            sub_format_error = false;
                            ++pos;//让pos指向}的下一个字符处, 去判断是]或者下一个要处理的格式化字符
                            break;
                        }
                        val.append(1, _pattern[pos++]);
                    }
                }

                //改动
                //原本是没有if判断, 直接写着arr.push_back, 这里加上了判断. 这里有3种情况
                //1、上面的if判断中, 如果子格式正确, 也就是以}结尾, 那么sub就是false, 这里就会设置进去
                //2、如果是pos越界退出的, 此时子格式的形式就不对, 也就是{... 那么sub就是true, 那这里就不设置进去
                //3、没有子格式, 没进if块中, sub_format_error就是默认的false，这里就会设置进去, 记录子格式用的val为默认的空
                if(!sub_format_error) arr.push_back(std::make_tuple(key, val, 1));

                //无论怎样, 两个都清空
                key.clear();
                val.clear();
            }

            //为true, 也就是没有找到}, 没改成false就走出循环, 字符串到头了, 说明子格式不对
            if (sub_format_error)
            {
                std::cout << "{}对应出错\n";
                return false;
            }

            //和while中的第一个if呼应
            if (string_row.empty() == false) arr.push_back(std::make_tuple(string_row, "", 0));

            //改动
            //原本是有这一句, 现在是去掉了这句
            //if (key.empty() == false) arr.push_back(std::make_tuple(key, val, 1));

            //这里需要再看一下while循环, 在退出while之前, key都会被清空
            //从while开始能够走到设置key那里, 说明没有越界; key之后, 没有退出整个while的情况, 走到最后key, val都会清空, 然后回到上面的while判断
            //此时如果越界了就退出整个while, key为空, 不越界就继续, 此时就还是上句的情况
            

            //2、根据解析得到的数据初始化格式化子项数组成员
            for (auto &it : arr)//按照上面的改动, 此时arr里的都是正确的格式化字符、格式化字符+子格式、非格式化字符
            {
                //非格式化字符: (string_row, "", 0)
                if (std::get<2>(it) == 0)//get<2>表示拿出第3个参数, 为0就是非格式化字符
                {
                    FormatItem::ptr fi(new OtherFormatItem(std::get<0>(it)));//拿出对应的string_row
                    _items.push_back(fi);
                }
                else//格式化字符: (key, val, 1)
                {
                    FormatItem::ptr fi = createItem(std::get<0>(it), std::get<1>(it));//传入key，val
                    if (fi.get() == nullptr)
                    {
                        std::cout << "没有对应的格式化字符: %" << std::get<0>(it) << std::endl;
                        return false;
                    }
                    _items.push_back(fi);
                }
            }
            return true;
        }
    
        //根据不同的格式化字符创建不同的格式化子项对象
        FormatItem::ptr createItem(const std::string& key, const std::string& val)
        {
            if (key == "m") return FormatItem::ptr(new MsgFormatItem(val));
            if (key == "p") return FormatItem::ptr(new LevelFormatItem(val));
            if (key == "d") return FormatItem::ptr(new TimeFormatItem(val));
            if (key == "f") return FormatItem::ptr(new FileFormatItem(val));
            if (key == "l") return FormatItem::ptr(new LineFormatItem(val));
            if (key == "t") return FormatItem::ptr(new ThreadFormatItem(val));
            if (key == "c") return FormatItem::ptr(new NameFormatItem(val));
            if (key == "T") return FormatItem::ptr(new TabFormatItem(val));
            if (key == "n") return FormatItem::ptr(new NLineFormatItem(val));
            return FormatItem::ptr();

            /*if(key == "m") return std::make_shared<MsgFormatItem>(val);
            if(key == "p") return std::make_shared<LevelFormatItem>(val);
            if(key == "d") return std::make_shared<TimeFormatItem>(val);
            if(key == "f") return std::make_shared<FileFormatItem>(val);
            if(key == "l") return std::make_shared<LineFormatItem>(val);
            if(key == "t") return std::make_shared<ThreadFormatItem>(val);
            if(key == "c") return std::make_shared<NameFormatItem>(val);
            if(key == "T") return std::make_shared<TabFormatItem>(val);
            if(key == "n") return std::make_shared<NLineFormatItem>(val);
            if(key.empty()) return std::make_shared<OtherFormatItem>(val);*/
        }
    private:
        std::string _pattern;//格式化规则字符串
        std::vector<FormatItem::ptr> _items;
    };
}

#endif
