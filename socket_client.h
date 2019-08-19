#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "io_service.h"
#include "reliable_bytestream_base.h"

typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_sp;

class connector : public one_to_one_observable<void, socket_sp>
{
public:
	connector(int port = 19234, std::string host = "127.0.0.1") : timer(treeipc_over_boost::get_io_service())
	{
		this->host = host;
		this->port = port;
	}

	virtual ~connector()
	{
		//
	}

	void connect()
	{
		printf("%s: %s %d\n", __func__, host.c_str(), port);
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
			if(do_reconnect)
			{
				reconnect();
			}
			return;
		}
		else
		{
			printf("%s: connected\n", __func__);
			notify(socket);
		}
	}
	
	void reconnect()
	{
		timer.expires_from_now(boost::posix_time::seconds(1));
		timer.async_wait(boost::bind(&connector::reconnect_handler, this));
	}
	
	void reconnect_handler()
	{
		printf("%s\n", __func__);
		connect();
	}
	
	void set_do_reconnect(bool do_reconnect)
	{
		this->do_reconnect = do_reconnect;
	}
	
private:
	boost::asio::deadline_timer timer;
	bool do_reconnect = false;
	std::string host = "127.0.0.1";
	int port = 19234;
};

class socket_client : public reliable_bytestream_base, public connector::listener
{
public:
	/*constructor*/				socket_client				();
	/*constructor*/				socket_client				(socket_sp sp);
	/*destructor*/				~socket_client				();

	size_t						write						(const void *data, size_t size);

	void						set_socket					(socket_sp);
	void						start						();
	void						set_connector				(connector *c);

private:
	socket_sp					socket;

	std::string					host;
	int							port;

	bool						is_connected = false;

	char						buffer[256] = {0};
	
	connector					*conn = nullptr;

	void						connect						();
	void						connect_handler				(const boost::system::error_code &error);
	void						data_ready					(const boost::system::error_code &e, size_t bytes_read);

	void						start_receive				();

	void						process_notification		(socket_sp);
};
