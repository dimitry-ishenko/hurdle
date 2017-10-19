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
down::down(const src::settings& settings, shared_part part) :
    util::logger("Down" + std::to_string(part->nr())),
    settings_(settings), part_(std::move(part))
{
    auto from = std::get<0>(part_->range()), to = std::get<1>(part_->range());
    if(part_->size())
    {
        info() << "skipping " << part_->size();
        from += part_->size();
    }
    if(from > to) throw std::invalid_argument("Invalid range");

    ////////////////////
    handle_ = curl_easy_init();

    curl_easy_setopt(handle_, CURLOPT_URL, settings_.url.data());
    curl_easy_setopt(handle_, CURLOPT_FAILONERROR, true);

    curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, &down::write);
    curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT, settings_.read_timeout.count());
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_LIMIT, 1);
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_TIME, settings_.read_timeout.count());

    auto from_to = std::to_string(from) + "-" + std::to_string(to);
    curl_easy_setopt(handle_, CURLOPT_RANGE, from_to.data());

    ////////////////////
    future_ = std::async(std::launch::async, &down::proc, this);
}

////////////////////////////////////////////////////////////////////////////////
down::~down() noexcept
{
    future_.wait();
    curl_easy_cleanup(handle_);
}

////////////////////////////////////////////////////////////////////////////////
bool down::done() const
{
    return future_.wait_for(secs(0)) == std::future_status::ready;
}

////////////////////////////////////////////////////////////////////////////////
void down::proc()
{
    CURLcode code = CURLE_OK;
    for(std::size_t count = 0; count < settings_.retry_count; ++count)
    {
        if(count) info() << "Retrying";

        code = curl_easy_perform(handle_);
        if(code == CURLE_OK) break;

        std::this_thread::sleep_for(settings_.retry_sleep);
    }
    if(code != CURLE_OK) throw std::runtime_error(curl_easy_strerror(code));
}

////////////////////////////////////////////////////////////////////////////////
size_t down::write(void* data, size_t size, size_t n, void* self)
{
    return static_cast<src::down*>(self)->part_->write(
        static_cast<const char*>(data), size * n
    );
}

////////////////////////////////////////////////////////////////////////////////
}
