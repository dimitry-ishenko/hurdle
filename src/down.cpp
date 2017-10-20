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
down::down(src::part& part) : util::logger("down " + std::to_string(part.nr())),
    part_(part)
{
    auto from = part_.from();
    if(part_.size())
    {
        info() << "skipping " << part_.size();
        from += part_.size();
    }
    if(from > part_.to()) throw std::invalid_argument("Invalid range");

    ////////////////////
    auto ctx = context::instance();
    handle_ = curl_easy_init();

    curl_easy_setopt(handle_, CURLOPT_URL, ctx->url.data());
    curl_easy_setopt(handle_, CURLOPT_FAILONERROR, true);

    curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, &down::write);
    curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT, ctx->read_timeout.count());
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_LIMIT, 1);
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_TIME, ctx->read_timeout.count());

    auto from_to = std::to_string(from) + "-" + std::to_string(part_.to());
    curl_easy_setopt(handle_, CURLOPT_RANGE, from_to.data());

    ////////////////////
    tp_ = clock::now();
    future_ = std::async(std::launch::async, &down::read, this);
}

////////////////////////////////////////////////////////////////////////////////
down::~down() noexcept
{
    if(future_.valid()) future_.wait();
    if(handle_) curl_easy_cleanup(handle_);
}

////////////////////////////////////////////////////////////////////////////////
bool down::done() const { return future_.wait_for(secs(0)) == std::future_status::ready; }

////////////////////////////////////////////////////////////////////////////////
offset down::speed() noexcept
{
    using namespace std::chrono;

    auto val = clock::now() - tp_;
    if(val < 1ms) return 0;

    auto piece = piece_.exchange(0);
    auto value = 1000 * piece / duration_cast<milliseconds>(val).count();

    tp_ += val;
    return value;
}

////////////////////////////////////////////////////////////////////////////////
void down::read()
{
    auto ctx = context::instance();

    CURLcode code = CURLE_OK;
    for(std::size_t count = 0; count < ctx->retry_count; ++count)
    {
        if(count) info() << "Retrying";

        code = curl_easy_perform(handle_);
        if(code == CURLE_OK) break;

        std::this_thread::sleep_for(ctx->retry_sleep);
    }
    if(code != CURLE_OK) throw std::runtime_error(curl_easy_strerror(code));
}

////////////////////////////////////////////////////////////////////////////////
size_t down::write(void* data, size_t size, size_t n, void* pvoid)
{
    auto self = static_cast<down*>(pvoid);

    offset total = size * n;
    self->piece_ += n;

    return self->part_.write(static_cast<const char*>(data), total);
}

////////////////////////////////////////////////////////////////////////////////
}
