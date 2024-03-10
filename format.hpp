#ifndef __M_FMT_H__
#define __M_FMT_H__

#include "message.hpp"
#include <vector>
#include <cassert>
#include <sstream>
#include <tuple>

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
    
    class TimeFormatItem : public FormatItem//Time has subformat
    {
    public:
        TimeFormatItem(const std::string& fmt = "%H:%M:%S"):_time_fmt(fmt)
        {
            if(fmt.empty()) _time_fmt = "%H:%M:%S";
        }

        void format(std::ostream& out, const LogMsg& msg) override
        {
            struct tm t;
            //Get the current system time from the first parameter, put it into t, and return tm structure pointer
            localtime_r(&msg._ctime, &t);
            char tmp[128] = {0};
            //Format the time in t according to the format and put it in tmp
            //The number represents the maximum number of characters copied to tmp, which must be greater than the number of format characters
            strftime(tmp, 127, _time_fmt.c_str(), &t);
            out << tmp;//Put in output stream
        }
    private:
        std::string _time_fmt;//The default is %H:%M:%S
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
        %d date, including subformats: {%H:%M:%S}
        %t thread id
        %c logger name
        %f source code file name
        %l source code line number
        %p log level
        %T tab indent
        %m body message
        %n newline
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

        //Overloaded, format the message and put string into IO stream
        std::ostream& format(std::ostream& out, const LogMsg& msg)
        {
            //Get the corresponding information from msg one by one in the order of parsing
            for(auto& item : _items)
            {
                //Each derived classes overloads format
                item->format(out, msg);
            }
            return out;
        }

        //Format message and return the formatted string
        std::string format(const LogMsg& msg)
        {
            std::stringstream ss;
            for(auto& it : _items) it->format(ss, msg);
            return ss.str();
        }
    private:
        //Parse the formatting rule string and add it to the array
        bool parsePattern()
        {
            //1、Parse the formatting rule string
            std::vector<std::tuple<std::string, std::string, int>> arr;//Tuple can't be modified
            //std::vector<std::pair<std::string, std::string>> fmt_order;

            size_t pos = 0;
            //Used to determine whether the subformat is obtained correctly
            bool sub_format_error = false;
            std::string key;//Store the formatting characters after %
            std::string val;//Store the subformat string in {} after the formatting character
            std::string string_row;//Store unformatted characters

            //Default string form: [%d{%H:%M:%S}][%t][%p][%c][%f:%l] %m%n
            while(pos < _pattern.size())
            {
                if(_pattern[pos] != '%')
                {
                    //++. After the loop goes up, it may cross the boundary and exit while
                    //At this point string_row already has content, so it needs to be processed after while ends.
                    string_row.append(1, _pattern[pos++]);
                    //string& append (size_t n, char c), Appends n consecutive copies of character c
                    continue;
                }
                //Here pos points to the first %

                //It is possible that %% occurs, and [%d%%%F%%F%%%%d] may occur
                //The rule here is that multiple formatting characters are allowed within a [], but their format must be like %z
                //Multiple % before z are regarded as one %
                while(pos + 1 < _pattern.size() && _pattern[pos + 1] == '%')
                    ++pos;
                //At this time, pos points to %. There must be a character after %
                //But it's not clear whether it is a specified formatting character.

                //Process unformatted characters first
                if(string_row.empty() == false)
                {
                    //Taking advantage of the unmodifiable feature of tuples, the val of the unformatted character is fixed to ""
                    //And then call OtherFormatItem
                    arr.push_back(std::make_tuple(string_row, "", 0));
                    string_row.clear();
                }

                ++pos;//pos points to the starting position of the character after %

                if (pos < _pattern.size() && isalpha(_pattern[pos]))
                {
                    key = _pattern[pos];//Save formatted characters
                }
                else//If there is no formatting character and it's not judged as % above, then error
                {
                    std::cout << "From here " << &_pattern[pos - 1] << " is malformed! \n";
                    return false;
                }
                //In other words, the above only judges %. As for the characters after %, leave it to ifelse here to handle it

                ++pos;//Check here to see if there is subformat, example: %d{...}, at this time pos is at {

                //Here we also need to judge whether we have crossed the line.
                if (pos < _pattern.size() && _pattern[pos] == '{')
                {
                    sub_format_error = true;
                    pos += 1;//pos points to after {, that is, the first character position of the subformat
                    while (pos < _pattern.size())
                    {
                        if (_pattern[pos] == '}')//Either {} or {...}, both forms are correct
                        {
                            sub_format_error = false;
                            //Let pos point to the next character of } to determine whether it is ] or the next formatting character to be processed
                            ++pos;
                            break;
                        }
                        val.append(1, _pattern[pos++]);
                    }
                }

                //Change
                //Originally there was no if, and arr.push_back was written directly. The judgment is added here. There are three situations here
                //1、In the above if judgment, if the sub format is correct, that is, it ends with }, then sub is false and will be set correctly
                //2、If pos exits out of bounds, the form of the sub format is incorrect, that is, {... Then sub is true, so it will not be set.
                //3、There is no subformat, and it doesn't enter the if block. The sub_format_error is false by default, and will be set here->
                //->The val used to record the subformat is NULL by default.
                if(!sub_format_error) arr.push_back(std::make_tuple(key, val, 1));

                //Either way, clear both
                key.clear();
                val.clear();
            }

            //Is true, that is, } is not found
            //If it's not changed to false, it exits the loop, the string ends
            //Indicate that the subformat is incorrect
            if (sub_format_error)
            {
                std::cout << "{} error\n";
                return false;
            }

            //Echoes the first if in while
            if (string_row.empty() == false) arr.push_back(std::make_tuple(string_row, "", 0));

            //Change
            //Originally there was this sentence, but now it has been removed
            //if (key.empty() == false) arr.push_back(std::make_tuple(key, val, 1));

            //Here we need to look at the while loop again. Before exiting the while, the key will be cleared
            //Starting from while, the code can run to where the key is set, indicating that there is no out of bounds
            //After the key, there is no exit from the entire while.vAt the end, the key and val will be cleared, then continue
            //At this time, if the boundary is crossed, exit the entire while, and key is empty
            //If it doesn't cross the boundary, continue. At this time, it's still the same situation as in the previous sentence
            

            //2、Use the parsed data to initialize the formatted child array members
            //According to the above changes, at this time
            //The contents in arr are correct formatting characters, or formatting characters + subformat, or non-formatting characters
            for (auto &it : arr)
            {
                //Non-formatting character: (string_row, "", 0)
                if (std::get<2>(it) == 0)//get<2> means taking out the third parameter. 0 means it's an unformatted character
                {
                    FormatItem::ptr fi(new OtherFormatItem(std::get<0>(it)));//Take out the corresponding string_row
                    _items.push_back(fi);
                }
                else//Format characters: (key, val, 1)
                {
                    FormatItem::ptr fi = createItem(std::get<0>(it), std::get<1>(it));//Pass in key， val
                    if (fi.get() == nullptr)
                    {
                        std::cout << "No corresponding formatting character: %" << std::get<0>(it) << std::endl;
                        return false;
                    }
                    _items.push_back(fi);
                }
            }
            return true;
        }
    
        //Create different formatting subkey objects based on different formatting characters
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
        std::string _pattern;//Formatting rule string
        std::vector<FormatItem::ptr> _items;
    };
}

#endif
