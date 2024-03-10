#ifndef __M_UTIL_H__
#define __M_UTIL_H__

/*  Utility tools:
    1、Get system time
    2、Determine whether the file exists
    3、Get file path
    4、Create directory
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
            static time_t now() {return time(nullptr); }//Get system time
        };

        class File
        {
        public:
            //Determine whether the file exists
            static bool exists(const std::string& name)
            {
                //Linux interface, filling in F_OK at the end can be used to check whether the file exists. If it does, return 0, if not, return -1
                //return (access(pathname.c_str(), F_OK) == 0);

                //Common interface (stat follows POSIX standard) 
                struct stat st;
                //Header: sys/stat.h, stat is used to obtain file attributes. If obtained, it means that file exists.
                return stat(name.c_str(), &st) == 0;
            }

            //Get file path
            static std::string path(const std::string& pathname)
            {
                if(pathname.empty()) return ".";
                //Find the last "/" before the file name /, /\\ is common in Windows and Linux
                size_t pos = pathname.find_last_of("/\\");
                if(pos == std::string::npos) return ".";//Can be seen in current directory
                return pathname.substr(0, pos + 1);//pos + 1  Includes "/"
            }
            
            //Create directory
            static void createDirectory(const std::string& path)
            {
                if(path.empty() || exists(path)) return ;//Exit if the parameter is empty or already exists
                size_t pos = 0, idx = 0;//idx: the location where the search starts
                while (idx < path.size())
                {
                    pos = path.find_first_of("/\\", idx);
                    if (pos == std::string::npos)//Header: sys/stat.h, sys/types.h
                    {
                        mkdir(path.c_str(), 0777);
                        return ;
                    }
                    if(pos == idx)
                    {
                        idx = pos + 1;
                        continue;
                    }

                    //The next sentence needs to get the complete path, so it starts from 0
                    //The following pos indicates the currently checked "/" position, and +1 includes "/"
                    //No +1 here, that is to say
                    //for example, /a/b, here we only take /a, create the a directory first, and after looping again, we will get /b
                    std::string subdir = path.substr(0, pos);
                    //std::cout << subdir << std::endl;

                    //Determine whether there is currently a subdir directory
                    //if (subdir == "." || subdir == "..") {idx = pos + 1; continue;}//Removed, because "exists" will also judge this

                    if(exists(subdir) == true)//. and .. both return true
                    {
                        idx = pos + 1;
                        continue;
                    }
                    //Create if it doesn't exist
                    mkdir(subdir.c_str(), 0777);
                    idx = pos + 1;
                }
            }
        };
    }
}

#endif
