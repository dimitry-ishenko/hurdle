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
    down(int nr, offset from, offset to);
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
        swap(future_, rhs.future_);
        swap(part_  , rhs.part_  );
        size_ = rhs.size_.exchange(size_);
        swap(total_ , rhs.total_ );
        piece_ = rhs.piece_.exchange(piece_);
        swap(tp_    , rhs.tp_    );
    }

    ////////////////////
    bool ready() const { return future_.wait_for(secs(0)) == std::future_status::ready; }
    part get() { return future_.get(); }

    offset size() const noexcept { return size_; }
    double done() const noexcept { return size() / total_; }

    offset speed() noexcept;

private:
    ////////////////////
    CURL* handle_ = nullptr;

    std::future<part> future_;
    part read(int nr, offset from, offset to);

    part part_;
    std::atomic<offset> size_;
    double total_;

    std::atomic<offset> piece_ { 0 };

    using clock = std::chrono::system_clock;
    clock::time_point tp_;

    static size_t write(void* data, size_t size, size_t n, void* pvoid);
};

////////////////////////////////////////////////////////////////////////////////
inline void swap(down& lhs, down& rhs) noexcept { lhs.swap(rhs); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
