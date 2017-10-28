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
head::head() : util::logger("head")
{
    auto ctx = context::instance();
    handle_ = curl_easy_init();

    info() << "url = " << ctx->url;
    curl_easy_setopt(handle_, CURLOPT_URL, ctx->url.data());
    curl_easy_setopt(handle_, CURLOPT_NOBODY, true);
    curl_easy_setopt(handle_, CURLOPT_FAILONERROR, true);

    curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT, ctx->timeout.count());
    curl_easy_setopt(handle_, CURLOPT_TIMEOUT, ctx->timeout.count());

    update();
}

////////////////////////////////////////////////////////////////////////////////
head::~head() noexcept { curl_easy_cleanup(handle_); }

////////////////////////////////////////////////////////////////////////////////
void head::update()
{
    auto code = curl_easy_perform(handle_);
    if(code) throw std::runtime_error(curl_easy_strerror(code));

    curl_off_t size;
    code = curl_easy_getinfo(handle_, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size);
    if(code) throw std::runtime_error(curl_easy_strerror(code));

    size_ = size;
    info() << "size = " << size_;
}

////////////////////////////////////////////////////////////////////////////////
}
