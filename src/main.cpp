////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "context.hpp"
#include "down_all.hpp"
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

void read_args(int argc, char* argv[]);
void version(const char*);
void usage(const char*);

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    int code = 0;
    curl_global_init(CURL_GLOBAL_ALL);

    try
    {
        read_args(argc, argv);

        code = src::down_all().run();
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
bool starts_with(const std::string& arg, const std::string& prefix, std::string& value)
{
    if(0 == arg.compare(0, prefix.size(), prefix))
    {
        value = arg.substr(prefix.size());
        return true;
    }
    else return false;
}

////////////////////////////////////////////////////////////////////////////////
int number(const std::string& value)
try
{
    std::size_t p = 0;
    int n = std::stoi(value, &p);
    if(p != value.size()) throw "stoi";
    return n;
}
catch(...) { throw invalid_argument("Invalid number"); }

////////////////////////////////////////////////////////////////////////////////
void read_args(int argc, char* argv[])
{
    auto ctx = src::context::instance();

    for(int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        std::string value;

             if(arg.empty()) continue;
        else if(arg == "-v" || arg == "--version") version(argv[0]), throw need_to_exit();
        else if(arg == "-h" || arg == "--help"   ) usage(argv[0]), throw need_to_exit();
        else if(arg == "-q" || arg == "--quiet"  ) util::send_to_console(false);
        else if(arg == "-o" || arg == "--output" )
        {
            if(i + 1 < argc) ctx->output = argv[++i];
            else throw invalid_argument("Missing output path");
        }
        else if(starts_with(arg, "--output=", ctx->output))
        {
            if(ctx->output.empty()) throw invalid_argument("Missing output path");
        }
        else if(arg == "-c" || arg == "--part-count" )
        {
            if(i + 1 < argc) ctx->part_count = number(argv[++i]);
            else throw invalid_argument("Missing number");
        }
        else if(starts_with(arg, "--part-count=", value))
        {
            if(value.size()) ctx->part_count = number(value);
            else throw invalid_argument("Missing number");
        }
        else if(arg == "-s" || arg == "--part-size" )
        {
            if(i + 1 < argc) ctx->part_size = number(argv[++i]);
            else throw invalid_argument("Missing number");
        }
        else if(starts_with(arg, "--part-size=", value))
        {
            if(value.size()) ctx->part_size = number(value);
            else throw invalid_argument("Missing number");
        }
        else if(arg[0] != '-') ctx->url = arg;
        else throw invalid_argument("Invalid argument: " + arg);
    }


    ////////////////////
    if(ctx->url.empty()) throw invalid_argument("Missing url");

    if(ctx->output.empty())
    {
        ctx->output = ctx->url;

        auto p = ctx->output.find_first_of('?');
        if(p != std::string::npos) ctx->output.resize(p);

        p = ctx->output.find_last_of('/');
        if(p != std::string::npos) ctx->output.erase(0, p + 1);
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
                 "    -c, --part-count=<n> Download <n> parts at the same time\n"
                 "    -s, --part-size=<n>  Download parts of <n> bytes in size\n"
              << std::endl;
}
