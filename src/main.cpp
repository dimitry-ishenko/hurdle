////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "settings.hpp"
#include "util/logging.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

#include <curl/curl.h>

////////////////////////////////////////////////////////////////////////////////
constexpr int major = 0;
constexpr int minor = 1;

struct invalid_argument : public std::invalid_argument
{ using std::invalid_argument::invalid_argument; };

struct need_to_exit : public std::exception { };

void read_args(int argc, char* argv[], src::settings&);
void version(const char*);
void usage(const char*);

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    int code = 0;
    curl_global_init(CURL_GLOBAL_ALL);

    try
    {
        src::settings settings;
        read_args(argc, argv, settings);

        //
    }
    catch(invalid_argument& e)
    {
        std::cout << e.what() << "\n" << std::endl;
        usage(argv[0]);
        code = 1;
    }
    catch(need_to_exit&) { }
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
void read_args(int argc, char* argv[], src::settings&)
{
    for(int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if(arg.empty()) continue;

        if(arg == "-v" || arg == "--version")
        {
            version(argv[0]); throw need_to_exit();
        }
        else if(arg == "-h" || arg == "--help")
        {
            usage(argv[0]); throw need_to_exit();
        }
        else if(arg == "-q" || arg == "--quiet")
        {
            util::send_to_console(false);
        }
        else throw invalid_argument("Invalid argument: " + arg);
    }
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
