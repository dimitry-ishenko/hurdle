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
part::part(int nr, offset from, offset to) :
    util::logger("part " + std::to_string(nr)),
    nr_(nr), from_(from), to_(to), path_(context::instance()->output)
{
    auto from_to = std::to_string(from_) + "-" + std::to_string(to_);
    info() << "range = " << from_to;

    auto p = path_.find_last_of('.');
    if(p == std::string::npos) p = path_.size();
    path_.insert(p, "_" + from_to);

    ////////////////////
    info() << "opening file " << path_;
    using std::ios_base;
    file_.open(path_, ios_base::out | ios_base::app | ios_base::binary);
    if(!file_) throw std::runtime_error("Part open failed");

    size_ = file_.tellp();
    total_ = to_ - from_;
}

////////////////////////////////////////////////////////////////////////////////
part::~part() noexcept
{
    info() << "closing file";
    file_.close();
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
void part::merge_to(std::fstream& file)
{
    std::unique_ptr<char[]> store(new char[size_]);

    file_.seekg(0, std::ios_base::beg);
    file_.read(store.get(), size_);
    if(!file_) throw std::runtime_error("Read failed");

    file.write(store.get(), size_);
    if(!file) throw std::runtime_error("Write failed");
}

////////////////////////////////////////////////////////////////////////////////
void part::remove() noexcept
{
    info() << "removing file";
    if(std::remove(path_.data())) err() << "Remove failed";
}

////////////////////////////////////////////////////////////////////////////////
}
