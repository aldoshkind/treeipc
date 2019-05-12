#pragma once

#include <queue>
#include <vector>
#include <mutex>

#include <QTcpSocket>

#include "../reliable_bytestream_base.h"

class connector : public one_to_one_observable<void>
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
		//
	}
};

class socket_bytestream : public QObject, public reliable_bytestream_base, public connector::listener
{
	Q_OBJECT

public:
	/*constructor*/				socket_bytestream				(QTcpSocket *s);
	/*destructor*/				~socket_bytestream				();

	size_t						write						(const void *data, size_t size);

	void						start						();

private:
	QTcpSocket					*socket = nullptr;

	std::mutex					send_buffer_mutex;
	std::vector<uint8_t>		send_buffer;

	std::string					host;
	int							port;

	bool						is_connected;

	char						buffer[256];

//	void						connect						();
	void						process_notification		();

	void						process_available_data		();

signals:
	void						signal_write_ready			();

private slots:
	void						slot_data_ready				();
	void						slot_write_ready			();
};
