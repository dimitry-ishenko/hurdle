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
    part(const settings&, int nr, offset from, offset to);
    ~part() noexcept;

    auto nr() const noexcept { return nr_; }
    auto from() const noexcept { return from_; }
    auto to() const noexcept { return to_; }

    // called from another thread
    offset write(const char*, offset);
    auto size() const noexcept { return size_.load(); }

    auto bucket() const noexcept { return bucket_.load(); }
    void reset_bucket() noexcept { bucket_ = 0; }

private:
    ////////////////////
    int nr_;
    offset from_, to_;

    std::string path_;
    std::fstream file_;

    std::atomic<offset> size_, bucket_;
};

////////////////////////////////////////////////////////////////////////////////
using shared_part = std::shared_ptr<part>;

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
