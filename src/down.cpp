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
down::down(const src::context& settings, shared_part part) :
    util::logger("down " + std::to_string(part->nr())),
    settings_(settings), part_(std::move(part))
{
    auto from = part_->from();
    if(part_->size())
    {
        info() << "skipping " << part_->size();
        from += part_->size();
    }
    if(from > part_->to()) throw std::invalid_argument("Invalid range");

    ////////////////////
    handle_ = curl_easy_init();

    curl_easy_setopt(handle_, CURLOPT_URL, settings_.url.data());
    curl_easy_setopt(handle_, CURLOPT_FAILONERROR, true);

    curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, &down::write);
    curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT, settings_.read_timeout.count());
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_LIMIT, 1);
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_TIME, settings_.read_timeout.count());

    auto from_to = std::to_string(from) + "-" + std::to_string(part_->to());
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
offset down::speed() noexcept
{
    using namespace std::chrono;

    auto interval = clock::now() - tp_;
    if(interval < 1ms) return 0;

    auto piece = piece_.exchange(0);
    auto value = 1000 * piece / duration_cast<milliseconds>(interval).count();

    tp_ += interval;
    return value;
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
size_t down::write(void* data, size_t size, size_t n, void* pvoid)
{
    auto self = static_cast<down*>(pvoid);

    offset total = size * n;
    self->piece_ += n;

    return self->part_->write(static_cast<const char*>(data), total);
}

////////////////////////////////////////////////////////////////////////////////
}
