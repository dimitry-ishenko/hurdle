////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef SRC_PART_HPP
#define SRC_PART_HPP

////////////////////////////////////////////////////////////////////////////////
#include "settings.hpp"
#include "util/logging.hpp"

#include <atomic>
#include <fstream>
#include <memory>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
class part : private util::logger
{
public:
    ////////////////////
    part(const settings&, int nr, src::range);
    ~part() noexcept;

    auto nr() const noexcept { return nr_; }
    auto const& range() const noexcept { return range_; }

    // called from another thread
    offset write(const char*, offset);
    auto size() const noexcept { return size_.load(); }

private:
    ////////////////////
    int nr_;
    src::range range_;

    std::string path_;
    std::fstream file_;

    std::atomic<offset> size_ { 0 };
};

////////////////////////////////////////////////////////////////////////////////
using shared_part = std::shared_ptr<part>;

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
