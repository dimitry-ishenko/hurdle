////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef SRC_DOWN_HPP
#define SRC_DOWN_HPP

////////////////////////////////////////////////////////////////////////////////
#include "context.hpp"
#include "part.hpp"
#include "util/logging.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <utility>

#include <curl/curl.h>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
class down;
using down_ptr = std::unique_ptr<down>;

////////////////////////////////////////////////////////////////////////////////
class down : private util::logger
{
public:
    ////////////////////
    down(int nr, offset from, offset to);
    ~down() noexcept;

    ////////////////////
    bool ready() const { return future_.wait_for(secs(0)) == std::future_status::ready; }
    part_ptr part() { return future_.get(); }

    offset size() const noexcept { return size_; }
    double done() const noexcept { return size() / total_; }

    offset speed() noexcept;

private:
    ////////////////////
    CURL* handle_ = nullptr;

    std::future<part_ptr> future_;
    part_ptr read(int nr, offset from, offset to);

    part_ptr part_;
    std::atomic<offset> size_;
    double total_;

    std::atomic<offset> piece_ { 0 };

    using clock = std::chrono::system_clock;
    clock::time_point tp_;

    static size_t write(void* data, size_t size, size_t n, void* pvoid);
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
