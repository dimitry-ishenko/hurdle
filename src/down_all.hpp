////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef SRC_DOWN_ALL_HPP
#define SRC_DOWN_ALL_HPP

////////////////////////////////////////////////////////////////////////////////
#include "down.hpp"
#include "head.hpp"
#include "output.hpp"
#include "part.hpp"
#include "util/logging.hpp"

#include <map>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
class down_all : private util::logger
{
public:
    ////////////////////
    down_all();

    ////////////////////
    int run();

private:
    ////////////////////
    head head_;
    output output_;

    int nr = 0;
    std::map<int, down> down_;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
