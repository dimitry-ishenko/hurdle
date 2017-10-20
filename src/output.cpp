////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "output.hpp"
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
output::output() : util::logger("out")
{
    auto ctx = context::instance();

    info() << "opening file " << ctx->output;
    file_.open(ctx->output, file_.out | file_.app | file_.binary);
    if(!file_) throw std::runtime_error("Output open failed");

    info() << "size = " << size();
}

////////////////////////////////////////////////////////////////////////////////
void output::merge(part_ptr part)
{
    parts_.emplace(part->nr(), std::move(part));

    while(parts_.begin() != parts_.end())
    {
        auto& part = std::get<1>(*parts_.begin());
        if(part->from() == size())
        {
            info() << "merging part " << part->nr();

            auto store = part->read_all();
            file_.write(store.data(), store.size());
            if(!file_) throw std::runtime_error("Failed to write part data");

            info() << "size = " << size();

            part->remove();
            parts_.erase(parts_.begin());
        }
        else if(part->from() > size()) break;
        else throw std::invalid_argument("Invalid part");
    }
}

////////////////////////////////////////////////////////////////////////////////
}
