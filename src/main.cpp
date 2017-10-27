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

void read_args(const std::string& name, char** args);
void version(const std::string& name);
void usage(const std::string& name);

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    int code = 0;
    curl_global_init(CURL_GLOBAL_ALL);

    try
    {
        read_args(argv[0], argv+1);
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
int number(const std::string& value)
try
{
    std::size_t p = 0;
    int n = std::stoi(value, &p);
    return p == value.size() ? n : 0;
}
catch(...) { return 0; }

////////////////////////////////////////////////////////////////////////////////
bool read_opt(char**& args, const std::string& short_opt, const std::string& long_opt, std::string& value)
{
    std::string arg = *args;

    if(arg == short_opt || arg == long_opt)
    {
        if(!*++args) throw invalid_argument("Missing value for option " + arg);

        value = *args;
        return true;
    }
    else if(0 == arg.compare(0, long_opt.size(), long_opt))
    {
        if(arg[long_opt.size()] == '=')
        {
            value = arg.substr(long_opt.size() + 1);
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
void read_args(const std::string& name, char** args)
{
    auto ctx = src::context::instance();

    for(; *args; ++args)
    {
        std::string arg = *args;
        std::string value;

             if(arg.empty()) continue;
        else if(arg == "-v" || arg == "--version") version(name), throw need_to_exit();
        else if(arg == "-h" || arg == "--help"   ) usage(name), throw need_to_exit();
        else if(arg == "-q" || arg == "--quiet"  ) util::send_to_console(false);
        else if(read_opt(args, "-o", "--output", ctx->output))
        {
            if(ctx->output.empty()) throw invalid_argument("Invalid output path");
        }
        else if(read_opt(args, "-c", "--part-count", value))
        {
            ctx->part_count = number(value);
            if(ctx->part_count < 1) throw invalid_argument("Invalid part count");
        }
        else if(read_opt(args, "-s", "--part-size", value))
        {
            ctx->part_size = number(value);
            if(ctx->part_size < 1) throw invalid_argument("Invalid part size");
        }
        else if(arg[0] != '-') ctx->url = arg;
        else throw invalid_argument("Invalid argument " + arg);
    }

    ////////////////////
    if(ctx->url.empty()) throw invalid_argument("Invalid or missing url");

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
void version(const std::string& name)
{
    std::cout << name << " version " << major << '.' << minor << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void usage(const std::string& name)
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
