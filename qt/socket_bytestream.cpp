#include "socket_bytestream.h"

/*constructor*/ socket_bytestream::socket_bytestream(QTcpSocket *s)
{
	socket = s;

	connect(this, SIGNAL(signal_write_ready()), this, SLOT(slot_write_ready()));
}

/*destructor*/ socket_bytestream::~socket_bytestream()
{
	//
}

size_t socket_bytestream::write(const void *data, size_t size)
{
	std::unique_lock<std::mutex> lock(send_buffer_mutex);
	send_buffer.insert(send_buffer.end(), (uint8_t *)data, (uint8_t *)data + size);
	lock.unlock();
	emit signal_write_ready();
#warning Lie
	return size;
}

void socket_bytestream::process_notification()
{
	//
}

void socket_bytestream::start()
{
	connect(socket, SIGNAL(readyRead()), this, SLOT(slot_data_ready()));
	if(socket->bytesAvailable() > 0)
	{
		process_available_data();
	}
}

void socket_bytestream::slot_data_ready()
{
	process_available_data();
}

void socket_bytestream::process_available_data()
{
	uint64_t rd = socket->read(buffer, sizeof(buffer));
	if(rd > 0)
	{
		//printf("data %s\n", buffer);
		notify(buffer, rd);
	}
}

void socket_bytestream::slot_write_ready()
{
	std::lock_guard<std::mutex> lock(send_buffer_mutex);
	socket->write((char *)&send_buffer[0], send_buffer.size());
	send_buffer.clear();
}
