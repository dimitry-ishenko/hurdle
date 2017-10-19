////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef SRC_DOWN_HPP
#define SRC_DOWN_HPP

////////////////////////////////////////////////////////////////////////////////
#include "part.hpp"
#include "settings.hpp"
#include "util/logging.hpp"

#include <atomic>
#include <chrono>
#include <future>

#include <curl/curl.h>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
class down : private util::logger
{
public:
    ////////////////////
    explicit down(const settings&, shared_part);
    ~down() noexcept;

    auto const& part() const noexcept { return part_; }
    bool done() const;

    offset speed() noexcept;

private:
    ////////////////////
    const settings& settings_;
    CURL* handle_;

    shared_part part_;

    std::future<void> future_;
    void proc();

    // access from another thread
    std::atomic<offset> piece_ { 0 };

    using clock = std::chrono::system_clock;
    clock::time_point tp_ = clock::now();

    static size_t write(void* data, size_t size, size_t n, void* pvoid);
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
