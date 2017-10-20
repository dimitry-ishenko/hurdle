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
output::output(const src::settings& settings) : util::logger("out")
{
    info() << "opening file " << settings.output;
    using std::ios_base;
    file_.open(settings.output, ios_base::out | ios_base::app | ios_base::binary);

    if(!file_) throw std::runtime_error("Output open failed");
}

////////////////////////////////////////////////////////////////////////////////
}
