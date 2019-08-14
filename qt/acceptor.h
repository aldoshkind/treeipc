#pragma once

#include <set>

#include <QTcpServer>

#include "../observable.h"
#include "socket_bytestream.h"
#include "package_stream.h"

class acceptor : public QObject, public one_to_one_observable<void, package_stream *>
{
	Q_OBJECT

public:
	acceptor()
	{
		tcpServer = new QTcpServer(this);
	}

	~acceptor()
	{
		//
	}

	void accept()
	{
		if (!tcpServer->listen(QHostAddress::Any, 33333) && server_status == false)
		{
			qDebug() <<  QObject::tr("Unable to start the server: %1.").arg(tcpServer->errorString());
		}
		else
		{
			server_status = true;
			connect(tcpServer, SIGNAL(newConnection()), this, SLOT(slot_new_connection()));
			qDebug() << QString::fromUtf8("Сервер запущен!");
		}
	}

	void connect_handler()
	{
		//notify(s);
	}

private slots:
	void slot_new_connection()
	{
		if(server_status == 1)
		{
			qDebug() << QString::fromUtf8("У нас новое соединение!");
			QTcpSocket *clientSocket = tcpServer->nextPendingConnection();
			clientSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

			socket_bytestream *sbs = new socket_bytestream(clientSocket);
			package_stream *ps = new package_stream(sbs);

			notify(ps);
		}
	}

private:
	QTcpServer *tcpServer = nullptr;
	bool server_status = false;
};


