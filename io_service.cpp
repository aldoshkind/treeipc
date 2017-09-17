#include "io_service.h"

#include <thread>
#include <functional>

using namespace boost::asio;

namespace treeipc_over_boost
{

class ios_wrapper
{
	io_service ios;
	std::thread thread;
	boost::asio::io_service::work work;

public:
	/*constructor*/					ios_wrapper					() : thread(std::bind(&ios_wrapper::run, this)), work(ios)
	{
		//
	}

	/*destructor*/					~ios_wrapper				()
	{
		ios.stop();
		thread.join();
	}

	void run()
	{
		ios.run();
	}

	io_service						&get_io_service				()
	{
		return ios;
	}
};

io_service &get_io_service()
{
	static ios_wrapper iow;
	return iow.get_io_service();
}

}
