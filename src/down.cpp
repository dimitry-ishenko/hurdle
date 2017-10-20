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

    curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT, ctx->read_timeout.count());
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_LIMIT, 1);
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_TIME, ctx->read_timeout.count());

    ////////////////////
    tp_ = clock::now();
    future_ = std::async(std::launch::async, &down::read, this, nr, from, to);
}

////////////////////////////////////////////////////////////////////////////////
down::~down() noexcept
{
    future_.wait();
    curl_easy_cleanup(handle_);
}

////////////////////////////////////////////////////////////////////////////////
offset down::speed() noexcept
{
    using msec = std::chrono::milliseconds;

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
    for(std::size_t count = 0; count < ctx->retry_count; ++count)
    {
        if(count) info() << "Retrying";

        ////////////////////
        part_ = std::make_unique<src::part>(nr, from, to);
        if(to - from + 1 == part_->size())
        {
            info() << "already done";
            return std::move(part_);
        }

        auto start = from;
        if(part_->size())
        {
            info() << "skipping " << part_->size();
            start += part_->size();
        }
        if(start > to) throw std::invalid_argument("Invalid range");

        size_ = part_->size();

        ////////////////////
        auto range = std::to_string(start) + "-" + std::to_string(to);
        curl_easy_setopt(handle_, CURLOPT_RANGE, range.data());

        code = curl_easy_perform(handle_);
        if(code == CURLE_OK) return std::move(part_);

        ////////////////////
        std::this_thread::sleep_for(ctx->retry_sleep);
    }

    throw std::runtime_error(curl_easy_strerror(code));
}

////////////////////////////////////////////////////////////////////////////////
size_t down::write(void* data, size_t size, size_t n, void* pvoid)
{
    auto self = static_cast<down*>(pvoid);

    offset total = size * n;
    self->size_ += n;
    self->piece_ += n;

    return self->part_->write(static_cast<const char*>(data), total);
}

////////////////////////////////////////////////////////////////////////////////
}
