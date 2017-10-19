////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "head.hpp"
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
head::head(const src::settings& settings) : util::logger("Head")
{
    handle_ = curl_easy_init();

    info() << "url = " << settings.url;
    curl_easy_setopt(handle_, CURLOPT_URL, settings.url.data());
    curl_easy_setopt(handle_, CURLOPT_NOBODY, true);
    curl_easy_setopt(handle_, CURLOPT_FAILONERROR, true);

    curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT, settings.read_timeout.count());
    curl_easy_setopt(handle_, CURLOPT_TIMEOUT, settings.read_timeout.count());

    ////////////////////
    auto code = curl_easy_perform(handle_);
    if(code) throw std::runtime_error(curl_easy_strerror(code));

    curl_off_t size;
    code = curl_easy_getinfo(handle_, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size);
    if(code) throw std::runtime_error(curl_easy_strerror(code));

    size_ = size;
    info() << "size = " << size_;
}

////////////////////////////////////////////////////////////////////////////////
head::~head() noexcept { curl_easy_cleanup(handle_); }

////////////////////////////////////////////////////////////////////////////////
}
