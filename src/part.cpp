////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "part.hpp"

#include <cstdio>
#include <stdexcept>
#include <thread>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
part::part(const src::settings& settings, int nr, const src::range& range) :
    util::logger("Part" + std::to_string(nr)),
    settings_(settings), path_(settings_.output)
{
    auto from = std::get<0>(range), to = std::get<1>(range);

    auto from_to = std::to_string(from) + "-" + std::to_string(to);
    info() << "range = " << from_to;

    auto p = path_.find_last_of('.');
    if(p == std::string::npos) p = path_.size();
    path_.insert(p, "_" + from_to);

    ////////////////////
    info() << "opening file " << path_;
    using std::ios_base;
    file_.open(path_, ios_base::out | ios_base::app | ios_base::binary);

    if(!file_) throw std::invalid_argument("Cannot open file");

    if(file_.tellp())
    {
        info() << "skipping " << file_.tellp();
        from += file_.tellp();
    }
    if(from > to) throw std::invalid_argument("Invalid range");

    ////////////////////
    handle_ = curl_easy_init();

    curl_easy_setopt(handle_, CURLOPT_URL, settings_.url.data());
    curl_easy_setopt(handle_, CURLOPT_FAILONERROR, true);

    curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, &part::write);
    curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT, settings_.read_timeout.count());
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_LIMIT, 1);
    curl_easy_setopt(handle_, CURLOPT_LOW_SPEED_TIME, settings_.read_timeout.count());

    curl_easy_setopt(handle_, CURLOPT_RANGE, from_to.data());

    ////////////////////
    future_ = std::async(std::launch::async, &part::proc, this);
}

////////////////////////////////////////////////////////////////////////////////
part::~part() noexcept
{
    future_.wait();
    curl_easy_cleanup(handle_);

    info() << "closing file";
    file_.close();

    info() << "removing file";
    std::remove(path_.data());
}

////////////////////////////////////////////////////////////////////////////////
bool part::done() const
{
    using namespace std::chrono_literals;
    return future_.wait_for(0s) == std::future_status::ready;
}

////////////////////////////////////////////////////////////////////////////////
void part::proc()
{
    CURLcode code = CURLE_OK;
    for(auto count = 0; count < settings_.retry_count; ++count)
    {
        if(count) info() << "Retrying";

        code = curl_easy_perform(handle_);
        if(code == CURLE_OK) break;

        std::this_thread::sleep_for(settings_.retry_sleep);
    }
    if(code != CURLE_OK) throw std::runtime_error(curl_easy_strerror(code));

    // merge
}

////////////////////////////////////////////////////////////////////////////////
size_t part::write(void* data, size_t size, size_t n, void* self)
{
    part* p = static_cast<part*>(self);
    auto total = size * n;

    p->file_.write(static_cast<const char*>(data), total);
    if(!p->file_) return 0;

    p->length_ = p->file_.tellp();
    return total;
}

////////////////////////////////////////////////////////////////////////////////
}
