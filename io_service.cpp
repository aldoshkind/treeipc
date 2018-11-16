#include "io_service.h"

using namespace boost::asio;

namespace treeipc_over_boost
{

io_service &get_io_service()
{
	static ios_wrapper iow;
	return iow.get_io_service();
}

}
