////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef SRC_PART_HPP
#define SRC_PART_HPP

////////////////////////////////////////////////////////////////////////////////
#include "context.hpp"
#include "util/logging.hpp"

#include <fstream>
#include <string>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
class part : private util::logger
{
public:
    ////////////////////
    part() = default;
    part(int nr, offset from, offset to);
    ~part() noexcept;

    part(const part&) = delete;
    part(part&& rhs) noexcept { swap(rhs); }

    part& operator=(const part&) = delete;
    part& operator=(part&& rhs) noexcept { swap(rhs); return *this; }

    void swap(part& rhs) noexcept
    {
        util::logger::operator=(std::move(rhs));

        using std::swap;
        swap(nr_   , rhs.nr_   );
        swap(from_ , rhs.from_ );
        swap(to_   , rhs.to_   );
        swap(path_ , rhs.path_ );
        swap(file_ , rhs.file_ );
    }

    ////////////////////
    auto nr() const noexcept { return nr_; }
    auto from() const noexcept { return from_; }
    auto to() const noexcept { return to_; }

    offset size() noexcept { return file_.tellp(); }

    ////////////////////
    offset write(const char*, offset);
    std::vector<char> read_all();

    void remove() noexcept;

private:
    ////////////////////
    int nr_; offset from_, to_;

    std::string path_;
    std::fstream file_;
};

////////////////////////////////////////////////////////////////////////////////
inline void swap(part& lhs, part& rhs) noexcept { lhs.swap(rhs); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
