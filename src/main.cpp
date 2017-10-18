////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "util/logging.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

#include <curl/curl.h>

////////////////////////////////////////////////////////////////////////////////
constexpr int major = 0;
constexpr int minor = 1;

enum status { good, done };
status read_args(int argc, char* argv[]);
void version(const char*);
void usage(const char*);

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    int code = 0;
    curl_global_init(CURL_GLOBAL_ALL);

    try
    {
        if(read_args(argc, argv) == good)
        {
            //
        }
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        code = 1;
    }
    catch(...)
    {
        std::cerr << "Unexpected error" << std::endl;
        code = 1;
    }

    curl_global_cleanup();
    return code;
}

////////////////////////////////////////////////////////////////////////////////
status read_args(int argc, char* argv[])
{
    for(int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if(arg.empty()) continue;

        if(arg == "-v" || arg == "--version")
        {
            version(argv[0]); return done;
        }
        else if(arg == "-h" || arg == "--help")
        {
            usage(argv[0]); return done;
        }
        else if(arg == "-q" || arg == "--quiet")
        {
            util::send_to_console(false);
        }
        else
        {
            std::cout << "Invalid argument: " << arg << "\n" << std::endl;
            usage(argv[0]); return done;
        }
    }
    return good;
}

////////////////////////////////////////////////////////////////////////////////
void version(const char* name)
{
    std::cout << name << " version " << major << '.' << minor << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void usage(const char* name)
{
    std::cout << "Usage: " << name << " [option...]\n" << std::endl;
    std::cout << "Where [option...] is one or more of the following:\n"
                 "    -v, --version        Show version info and exit\n"
                 "    -h, --help           Show this help screen and exit\n"
                 "    -q, --quiet          Don't output anything\n"
              << std::endl;
}
