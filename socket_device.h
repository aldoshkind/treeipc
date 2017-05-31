#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <map>

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "device.h"


class socket_device : public device
{
	std::thread					*thread_listener;

	std::string					host;
	int							port;

	uint8_t						pack_size;
	std::vector<uint8_t>		pack;

	msgid_t						current_msgid;

	std::mutex					senders_mutex;
	typedef std::map<msgid_t, std::condition_variable>	sender_condvars_t;
	sender_condvars_t			sender_condvars;

	package						in;

	void						process_package				(const package_t &p)
	{
		if(p.get_msgid() != 0)
		{
			std::lock_guard<std::mutex> lock(senders_mutex);
			in = p;
			sender_condvars[p.get_msgid()].notify_one();
		}

		accept_data(p);
	}

	void						push						(void *data, uint32_t size)
	{
		static bool wait_size = true;
		uint32_t available = size;
		for( ; available != 0 ; )
		{
			uint32_t bytes_to_wait = ((wait_size == true) ? sizeof(pack_size) : pack_size);
			uint32_t left = bytes_to_wait - pack.size();
			uint32_t copyable = std::min(available, left);

			pack.insert(pack.end(), (uint8_t *)data + (size - available), (uint8_t *)data + (size - available) + copyable);
			available -= copyable;
			left -= copyable;

			if(left == 0)
			{
				if(wait_size == true)
				{
					pack_size = *(decltype(pack_size) *)(&pack[0]);
				}
				else
				{
					printf("package of size %d got %s\n", pack_size, &pack[0]);

					package_t p;
					p.append(&pack[0], pack.size());

					process_package(p);

					pack_size = 0;
				}
				wait_size = !wait_size;
				pack.resize(0);
			}
		}
	}

	int connection_descriptor;

	void						listen_thread				()
	{
		for( ; ; )
		{
			int socket_descriptor = -1;
			connection_descriptor = -1;
			socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
			if(socket_descriptor < 0)
			{
				printf("failed to create socket\n");
				sleep(1);
				continue;
			}
			printf("socket opened\n");

			int yes = 1;
			if(setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
			{
				printf("failed to setsockopt socket\n");
				shutdown(socket_descriptor, SHUT_RDWR);
				close(socket_descriptor);
				sleep(1);
				continue;
			}

			sockaddr_in serv_addr;
			memset(&serv_addr, 0, sizeof(serv_addr));

			serv_addr.sin_family = AF_INET;
			if(host != "")
			{
				serv_addr.sin_addr.s_addr = inet_addr(host.c_str());
			}
			else
			{
				serv_addr.sin_addr.s_addr = INADDR_ANY;
			}
			serv_addr.sin_port = htons(port);

			if(bind(socket_descriptor, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
			{
				printf("failed to bind\n");
				shutdown(socket_descriptor, SHUT_RDWR);
				close(socket_descriptor);
				sleep(1);
				continue;
			}
			printf("socket bound\n");

			::listen(socket_descriptor, 1);

			printf("listen done\n");

			sockaddr_in client_addr;
			socklen_t len = sizeof(client_addr);
			connection_descriptor = accept(socket_descriptor, (struct sockaddr *)&client_addr, &len);
			printf("connection accepted\n");
			if(connection_descriptor == -1)
			{
				printf("failed accepting connection\n");
				shutdown(socket_descriptor, SHUT_RDWR);
				close(socket_descriptor);
				continue;
			}
			else
			{
				printf("connected\n");
			}

			for( ; ; )
			{
				char buff[512] = {0};
				int rd = read(connection_descriptor, buff, sizeof(buff));
				printf("socket read %d\n", rd);

				if(rd > 0)
				{
					printf("read %d\n", rd);
					push(buff, rd);
				}
				else
				{
					break;
				}
			}
			shutdown(connection_descriptor, SHUT_RDWR);
			close(connection_descriptor);
			connection_descriptor = -1;
			shutdown(socket_descriptor, SHUT_RDWR);
			close(socket_descriptor);
			sleep(1);
		}
		return;
	}

public:
	/*constructor*/				socket_device				()
	{
		thread_listener = NULL;
		current_msgid = 1;
	}

	/*destructor*/				~socket_device				()
	{
		if(thread_listener != NULL)
		{
			thread_listener->join();
			delete thread_listener;
		}
	}

	bool						write						(const package_t &p)
	{
		if(connection_descriptor == -1)
		{
			return false;
		}
		int rv = ::write(connection_descriptor, &p[0], p.size());
		if(rv != p.size())
		{
			return false;
		}
		return true;
	}

	bool						send						(package_t req, package_t &resp)
	{
		std::unique_lock<decltype(senders_mutex)> lock(senders_mutex);
		msgid_t mid = current_msgid++;
		req.set_msgid(mid);

		sender_condvars[mid].wait(lock);

		return false;
	}

	void						listen_on					(std::string host, int port)
	{
		this->host = host;
		this->port = port;

		if(thread_listener != NULL)
		{
			thread_listener->join();
			delete thread_listener;
		}

		thread_listener = new std::thread(&socket_device::listen_thread, this);
	}
};
