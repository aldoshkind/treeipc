#pragma once

#include <thread>
#include <functional>

#include <boost/asio/io_service.hpp>

namespace treeipc_over_boost
{

class ios_wrapper
{
	boost::asio::io_service ios;
	boost::asio::io_service::work work;
	std::thread thread;

public:
	/*constructor*/					ios_wrapper					() : work(ios), thread(std::bind(&ios_wrapper::run, this))
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

	boost::asio::io_service			&get_io_service				()
	{
		return ios;
	}
};

boost::asio::io_service &get_io_service();

}
