#include "socket_client.h"

#include "boost/bind.hpp"

#include "io_service.h"

/*constructor*/ socket_client::socket_client()
{
	is_connected = false;
}

/*constructor*/ socket_client::socket_client(socket_sp socket)
{
	set_socket(socket);
}

/*destructor*/ socket_client::~socket_client()
{
	//
}



size_t socket_client::write(const void *data, size_t size)
{
	if(is_connected == false)
	{
		return 0;
	}

	boost::system::error_code ec;
	size_t written = socket->write_some(boost::asio::buffer(data, size), ec);

	printf("written %d\n", written);

	if(ec.value() != 0)
	{
		printf("%s %s %d\n", __PRETTY_FUNCTION__, ec.message().c_str(), socket->is_open());
		is_connected = false;
		return 0;
	}

	return written;
}

void socket_client::data_ready(const boost::system::error_code &e, size_t bytes_read)
{
	printf("read %d\n", bytes_read);

	if(e.value() != 0)
	{
		printf("%s %s %d\n", __PRETTY_FUNCTION__, e.message().c_str(), socket->is_open());
		is_connected = false;
		return;
	}

	notify(buffer, bytes_read);

	start_receive();
}

void socket_client::process_notification(socket_sp ssp)
{
	set_socket(ssp);
}

void socket_client::start_receive()
{
	socket->async_receive(boost::asio::buffer(buffer, sizeof(buffer)),
						 0,
						 boost::bind(&socket_client::data_ready,
									 this,
									 boost::asio::placeholders::error,
									 boost::asio::placeholders::bytes_transferred
									 )
						 );
}

void socket_client::set_socket(socket_sp s)
{
	socket = s;
	is_connected = true;
	start_receive();
}
