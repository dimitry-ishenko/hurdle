////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef SRC_PART_HPP
#define SRC_PART_HPP

////////////////////////////////////////////////////////////////////////////////
#include "settings.hpp"
#include "util/logging.hpp"

#include <atomic>
#include <fstream>
#include <future>
#include <ios>
#include <string>
#include <tuple>

#include <curl/curl.h>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

using offset = std::streamoff;
using range = std::tuple<offset, offset>;

////////////////////////////////////////////////////////////////////////////////
class part : private util::logger
{
public:
    ////////////////////
    part(const settings&, int nr, const range&);
    ~part() noexcept;

    auto length() const noexcept { return length_.load(); }
    bool done() const;

private:
    ////////////////////
    const settings& settings_;
    CURL* handle_;

    std::string path_;
    std::fstream file_;
    std::atomic<offset> length_ { 0 };

    std::future<void> future_;
    void proc();

    static size_t write(void* data, size_t size, size_t n, void* self);
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
