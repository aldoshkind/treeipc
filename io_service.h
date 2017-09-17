#pragma once

#include <boost/asio/io_service.hpp>

namespace treeipc_over_boost
{

boost::asio::io_service &get_io_service();

}
