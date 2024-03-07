#ifndef __M_UTIL_H__
#define __M_UTIL_H__

/* 实用工具类的实现:
    1、获取系统时间
    2、判断文件是否存在
    3、获取文件所在路径
    4、创建目录
*/

#include <iostream>
#include <ctime>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

namespace Logs
{
    namespace LogUtil
    {
        class Date
        {
        public:
            static time_t now() {return time(nullptr); }//获取系统时间
        };

        class File
        {
        public:
            //判断文件是否存在
            static bool exists(const std::string& name)
            {
                //return (access(pathname.c_str(), F_OK) == 0);//Linux下的接口, 后面填F_OK可以用来检测文件有没有, 有就返回0, 没有返回-1; F_OK表示检查文件是否存在

                //通用接口(stat遵循POSIX标准) 
                struct stat st;
                //头文件sys/stat.h，stat用来获取文件属性, 获取得到就说明存在
                return stat(name.c_str(), &st) == 0;
            }

            //获取文件所在路径
            static std::string path(const std::string& pathname)
            {
                if(pathname.empty()) return ".";
                //找文件名之前的最后一个文件分隔符/, /\\是Windows, Linux通用写法, 查找斜杠和反斜杠中任意一个, 所以通用
                size_t pos = pathname.find_last_of("/\\");
                if(pos == std::string::npos) return ".";//说明在当前目录
                return pathname.substr(0, pos + 1);//pos + 1包含斜杠位置
            }
            
            //创建目录
            static void createDirectory(const std::string& path)
            {
                if(path.empty() || exists(path)) return ;//参数为空或者已经存在了就退出
                size_t pos = 0, idx = 0;//idx表示查找开始的位置
                while (idx < path.size())
                {
                    pos = path.find_first_of("/\\", idx);
                    if (pos == std::string::npos)//头文件sys/stat.h, sys/types.h
                    {
                        mkdir(path.c_str(), 0777);
                        return ;
                    }
                    if(pos == idx)//这里表明已经结束了
                    {
                        idx = pos + 1;
                        continue;
                    }

                    //下句要拿到完整的路径, 所以要从0开始, 后面的pos表示就是当前检查到的斜杠位置, +1包含斜杠
                    //这里不+1, 也就是说, 比如/a/b, 这里就只取/a, 先创建a目录, 再次循环后会得到/b
                    std::string subdir = path.substr(0, pos);
                    //std::cout << subdir << std::endl;

                    //先判断当前有没有subdir这个目录
                    //if (subdir == "." || subdir == "..") {idx = pos + 1; continue;}//去掉了, 因为下句exists也会判断这个
                    if(exists(subdir) == true)//.和..都会返回true 
                    {
                        idx = pos + 1;
                        continue;
                    }
                    //不存在就创建
                    mkdir(subdir.c_str(), 0777);
                    idx = pos + 1;
                }
            }
        };
    }
}

#endif
