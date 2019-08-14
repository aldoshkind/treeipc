#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "io_service.h"
#include "reliable_bytestream_base.h"

typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_sp;

class connector : public one_to_one_observable<void, socket_sp>
{
public:
	connector()
	{
		//
	}

	~connector()
	{
		//
	}

	void connect(std::string host, int port)
	{
		socket_sp socket(new socket_sp::element_type(treeipc_over_boost::get_io_service()));

		socket->async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), port),
							 boost::bind(&connector::connect_handler,
										this,
										boost::asio::placeholders::error,
										socket
										)
							 );
	}

	void connect_handler(const boost::system::error_code &error, socket_sp socket)
	{
		if(error)
		{
			printf("%s %s %d\n", __PRETTY_FUNCTION__, error.message().c_str(), socket->is_open());
			return;
		}
		else
		{
			notify(socket);
		}
	}
};

class socket_client : public reliable_bytestream_base, public connector::listener
{
	socket_sp					socket;

	std::string					host;
	int							port;

	bool						is_connected;

	char						buffer[256];

	void						connect						();
	void						connect_handler				(const boost::system::error_code &error);
	void						data_ready					(const boost::system::error_code &e, size_t bytes_read);

	void						start_receive				();

	void						process_notification		(socket_sp);

public:
	/*constructor*/				socket_client				();
	/*constructor*/				socket_client				(socket_sp sp);
	/*destructor*/				~socket_client				();

	size_t						write						(const void *data, size_t size);

	void						set_socket					(socket_sp);
	void						start						();
};
