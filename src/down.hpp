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
#include <utility>

#include <curl/curl.h>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
class down : private util::logger
{
public:
    ////////////////////
    down() = default;
    explicit down(part&);
    ~down() noexcept;

    down(const down&) = delete;
    down(down&& rhs) noexcept { swap(rhs); }

    down& operator=(const down&) = delete;
    down& operator=(down&& rhs) noexcept { swap(rhs); return *this; }

    void swap(down& rhs) noexcept
    {
        util::logger::operator=(std::move(rhs));

        using std::swap;
        swap(handle_, rhs.handle_);
        swap(part_  , rhs.part_  );
        swap(future_, rhs.future_);
        piece_ = rhs.piece_.exchange(piece_);
        swap(tp_    , rhs.tp_    );
    }

    ////////////////////
    bool done() const;
    offset speed() noexcept;

private:
    ////////////////////
    CURL* handle_ = nullptr;

    static part none_;
    part& part_ = none_;

    std::future<void> future_;
    void read();

    std::atomic<offset> piece_ { 0 };

    using clock = std::chrono::system_clock;
    clock::time_point tp_;

    static size_t write(void* data, size_t size, size_t n, void* pvoid);
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
