////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "down.hpp"

#include <stdexcept>
#include <thread>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
down::down(int nr, offset from, offset to) :
    util::logger("down " + std::to_string(nr)),
    total_(to - from + 1)
{
    auto ctx = context::instance();
    handle_ = curl_easy_init();

    curl_easy_setopt(handle_, CURLOPT_URL, ctx->url.data());
    curl_easy_setopt(handle_, CURLOPT_FAILONERROR, true);

    curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, &down::write);
    curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT, ctx->timeout.count());
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_LIMIT, 1);
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_TIME, ctx->timeout.count());

    ////////////////////
    tp_ = clock::now();
    future_ = std::async(std::launch::async, &down::read, this, nr, from, to);
}

////////////////////////////////////////////////////////////////////////////////
down::~down() noexcept
{
    if(future_.valid()) future_.wait();
    curl_easy_cleanup(handle_);
}

////////////////////////////////////////////////////////////////////////////////
offset down::speed() noexcept
{
    auto val = clock::now() - tp_;
    if(val < msec(1)) return 0;

    auto piece = piece_.exchange(0);
    tp_ += val;

    return 1000 * piece / std::chrono::duration_cast<msec>(val).count();
}

////////////////////////////////////////////////////////////////////////////////
part_ptr down::read(int nr, offset from, offset to)
{
    auto ctx = context::instance();
    CURLcode code = CURLE_OK;
    std::size_t retry = 0;

    goto start;
    for(;; ++retry)
    {
        if(retry > ctx->retry) throw std::runtime_error(curl_easy_strerror(code));

        part_.reset();
        std::this_thread::sleep_for(ctx->retry_time);
        info() << "retrying";

        ////////////////////
    start:
        part_ = std::make_unique<src::part>(nr, from, to);
        size_ = part_->size();

        if(to - from + 1 == size_)
        {
            info() << "already done";
            break;
        }

        auto start = from;
        if(size_)
        {
            info() << "skipping " << size_;
            start += size_;
        }
        if(start > to) throw std::invalid_argument("Invalid range");

        ////////////////////
        auto range = std::to_string(start) + "-" + std::to_string(to);
        curl_easy_setopt(handle_, CURLOPT_RANGE, range.data());

        code = curl_easy_perform(handle_);
        if(code == CURLE_OK)
        {
            info() << "done";
            break;
        }
        else if(cancel_)
        {
            info() << "cancel";
            part_.reset();
            break;
        }
    }

    return std::move(part_);
}

////////////////////////////////////////////////////////////////////////////////
size_t down::write(void* data, size_t size, size_t n, void* pvoid)
{
    auto self = static_cast<down*>(pvoid);
    if(self->cancel_) return 0;

    offset total = size * n;
    self->size_ += n;
    self->piece_ += n;

    return self->part_->write(static_cast<const char*>(data), total);
}

////////////////////////////////////////////////////////////////////////////////
}
