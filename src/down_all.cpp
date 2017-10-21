////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "down_all.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
down_all::down_all() : util::logger("down")
{
    size_ = output_.size();
}

////////////////////////////////////////////////////////////////////////////////
namespace
{

std::string pctage(double value)
{
    std::ostringstream os;
    os << std::fixed << std::setprecision(1) << 100 * value << "%";
    return os.str();
}

std::string scaled(double value)
{
    auto scale =
        value >= 1000 ?
            (value /= 1000) >= 1000 ?
                (value /= 1000) >= 1000 ?
                    (value /= 1000, "G/s")
                : "M/s"
            : "K/s"
        : "B/s";

    std::ostringstream os;
    os << std::fixed << std::setprecision(1) << value << scale;
    return os.str();
}

}

////////////////////////////////////////////////////////////////////////////////
int down_all::run()
{
    info() << "starting";

    auto ctx = context::instance();
    while(output_.size() < head_.size())
    {
        ////////////////////
        // add new parts
        while(size_ < head_.size() && downs_.size() < ctx->part_count)
        {
            auto end = size_ - (size_ % ctx->part_size) + ctx->part_size;
            if(end > head_.size()) end = head_.size();

            downs_.emplace(nr, std::make_unique<down>(nr, size_, end - 1));

            size_ = end;
            ++nr;
        }

        ////////////////////
        // show status
        std::ostringstream os;

        double speed_total = 0, size_total = output_.size();
        for(auto& pair : downs_)
        {
            int nr = std::get<0>(pair);
            auto& down = std::get<1>(pair);
            double speed = down->speed();

            os << std::setw(3) << nr << ": ";
            os << std::setw(6) << pctage(down->done()) << " ";
            os << std::setw(8) << scaled(speed) << " ";

            speed_total += speed;
            size_total += down->size();
        }

        os << " all: ";
        os << std::setw(6) << pctage(size_total / head_.size()) << " ";
        os << std::setw(8) << scaled(speed_total) << " ";

        info() << os.str();

        ////////////////////
        // merge done parts
        for(auto fi = downs_.begin(); fi != downs_.end();)
        {
            auto& down = std::get<1>(*fi);
            if(down->ready())
            {
                output_.merge(down->part());
                fi = downs_.erase(fi);
            }
            else ++fi;
        }

        ////////////////////
        std::this_thread::sleep_for(secs(1));
    }

    info() << "done";
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
}
