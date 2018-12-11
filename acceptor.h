#pragma once

#include <set>
#include <thread>

#include <boost/asio.hpp>

#include "observable.h"
#include "object_status.h"
#include "socket_client.h"

class acceptor : public one_to_one_observable<void, socket_sp>
{
	boost::asio::ip::tcp::acceptor acc;

	std::mutex mutex;

	object_status::sp status;

public:
	acceptor(boost::asio::ip::tcp::endpoint ep = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 13233))
				: acc(treeipc_over_boost::get_io_service(), ep)
				, status(new object_status)
	{
		acc.set_option(boost::asio::socket_base::reuse_address(true));
		acc.listen();
	}

	~acceptor()
	{
		acc.cancel();
		acc.close();
		std::lock_guard<decltype(status->mut)> lg(status->mut);
		printf("%s %p start\n", __PRETTY_FUNCTION__, this);
		status->ok = false;
		printf("%s %p end\n", __PRETTY_FUNCTION__, this);
	}

	void accept()
	{
		socket_sp socket(new socket_sp::element_type(treeipc_over_boost::get_io_service()));		
		
/*#warning Это костыль, тут утекает память и потоки
		auto iow = new treeipc_over_boost::ios_wrapper;*/
		
		//socket_sp socket(new socket_sp::element_type(iow->get_io_service()));

		auto f = boost::bind(&acceptor::connect_handler, this, boost::asio::placeholders::error, socket, status);

		acc.async_accept(*socket.get(), f);
	}

	void connect_handler(const boost::system::error_code &/*error*/, socket_sp s, object_status::sp status)
	{
		std::lock_guard<std::mutex> lg(status->mut);
		if(status->is_ok() == false)
		{
			return;
		}
		accept();
		notify(s);
	}
};


