////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "context.hpp"
#include <memory>

////////////////////////////////////////////////////////////////////////////////
src::context* src::context::instance()
{
    static std::unique_ptr<context> value;
    if(!value) value = std::make_unique<context>();
    return value.get();
}
