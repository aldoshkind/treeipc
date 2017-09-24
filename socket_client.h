#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "reliable_serial.h"

typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_sp;

/*class sock
{
	socket_sp sck;

public:
	sock(socket_sp s) : sck(s)
	{
		//
	}

	~sock()
	{
		//
	}

	boost::function<void(sock *)> on_ready;
	boost::function<void(sock *)> on_eof;

	void data_ready(const boost::system::error_code &e)
	{
		printf("%s %p\n", __PRETTY_FUNCTION__, this);
		if(on_ready)
		{
			on_ready(this);
		}
	}

	void begin_async_read()
	{
		sck->async_receive(boost::asio::null_buffers(), 0, boost::bind(&sock::data_ready, this, boost::asio::placeholders::error));
	}

	size_t write(const void *d, size_t size)
	{
		return sck->write_some(boost::asio::buffer(d, size));
	}

	size_t read(void *d, size_t size)
	{
		boost::system::error_code ec;
		size_t rv = sck->read_some(boost::asio::buffer(d, size), ec);

		if(ec.value() != 0)
		{
			printf("%s %p\n", __PRETTY_FUNCTION__, this);
			if(on_eof)
			{
				on_eof(this);
			}
		}
		else
		{
			begin_async_read();
		}
		return rv;
	}
};*/

#include "io_service.h"

typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_sp;

class connector : public one_to_one_observable<void, std::shared_ptr<boost::asio::ip::tcp::socket>>
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
	}
};

class socket_client : public reliable_serial, connector::listener
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
};
