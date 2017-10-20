////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "part.hpp"

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
part::part(int nr, offset from, offset to) : util::logger("part " + std::to_string(nr)),
    nr_(nr), from_(from), to_(to)
{
    auto from_to = std::to_string(from_) + "-" + std::to_string(to_);
    info() << "range = " << from_to;

    path_ = context::instance()->output;
    auto p = path_.find_last_of('.');
    if(p == std::string::npos) p = path_.size();
    path_.insert(p, "_" + from_to);

    info() << "opening file " << path_;
    file_.open(path_, file_.out | file_.app | file_.binary);
    if(!file_) throw std::runtime_error("Failed to open part file");

    size_ = file_.tellp();
    total_ = to_ - from_ + 1;
}

////////////////////////////////////////////////////////////////////////////////
part::~part() noexcept
{
    if(file_.is_open())
    {
        info() << "closing file";
        file_.close();
    }
}

////////////////////////////////////////////////////////////////////////////////
offset part::write(const char* data, offset n)
{
    if(file_.write(data, n))
    {
        size_ = file_.tellp();
        return n;
    }
    else return 0;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<char> part::read_all()
{
    std::vector<char> store(size_);

    file_.seekg(0, file_.beg);
    file_.read(store.data(), store.size());
    if(!file_) throw std::runtime_error("Failed to read part data");

    return store;
}

////////////////////////////////////////////////////////////////////////////////
void part::remove() noexcept
{
    info() << "removing file";
    if(std::remove(path_.data())) err() << "Failed to remove file";
}

////////////////////////////////////////////////////////////////////////////////
}
