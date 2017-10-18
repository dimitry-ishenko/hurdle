////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "down_all.hpp"
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

        code = src::down_all(settings).run();
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
void read_args(int argc, char* argv[], src::settings& settings)
{
    for(int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

             if(arg.empty()) continue;
        else if(arg == "-v" || arg == "--version") version(argv[0]), throw need_to_exit();
        else if(arg == "-h" || arg == "--help"   ) usage(argv[0]), throw need_to_exit();
        else if(arg == "-q" || arg == "--quiet"  ) util::send_to_console(false);
        else if(arg == "-o" || arg == "--output" )
        {
            if(i + 1 < argc) settings.output = argv[++i];
            else throw invalid_argument("Missing output path");
        }
        else if(0 == arg.compare(0, 9, "--output="))
        {
            if(arg.size() > 9) settings.output = arg.substr(9);
            else throw invalid_argument("Missing output path");
        }
        else if(arg[0] != '-') settings.url = arg;
        else throw invalid_argument("Invalid argument: " + arg);
    }


    ////////////////////
    if(settings.url.empty()) throw invalid_argument("Missing url");

    if(settings.output.empty())
    {
        settings.output = settings.url;

        auto p = settings.output.find_first_of('?');
        if(p != std::string::npos) settings.output.resize(p);

        p = settings.output.find_last_of('/');
        if(p != std::string::npos) settings.output.erase(0, p + 1);
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
    std::cout << "Usage: " << name << " [option...] <url>\n" << std::endl;
    std::cout << "Where [option...] is one or more of the following:\n"
                 "    -v, --version        Show version info and exit\n"
                 "    -h, --help           Show this help screen and exit\n"
                 "    -q, --quiet          Don't output anything\n"
                 "    -o, --output=<path>  Output data to <path>\n"
              << std::endl;
}
