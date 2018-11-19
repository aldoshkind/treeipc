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

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "device.h"
#include "io_service.h"

#include <boost/asio.hpp>

#include "reliable_serial.h"

class package_codec : public one_to_one_observable<void, const std::vector<uint8_t> &>, public reliable_serial::listener
{
	reliable_serial *transport;

	//package buffer;
	std::vector<uint8_t> buffer;
	typedef uint32_t pack_size_t;
	pack_size_t pack_size;

	void process_notification(const void *data, size_t size)
	{
		buffer.insert(buffer.end(), (char *)data, (char *)data + size);

		for( ; ; )
		{
			if((pack_size == 0) && (buffer.size() >= sizeof(pack_size)))
			{
				pack_size = *(pack_size_t *)(&buffer[0]);
				buffer.erase(buffer.begin(), buffer.begin() + sizeof(pack_size));
			}

			if(pack_size == 0 || buffer.size() < pack_size)
			{
				// ждём пока не наберётся достаточно данных
				return;
			}

			std::vector<uint8_t> result(buffer.begin(), buffer.begin() + pack_size);
			notify(result);
			buffer.erase(buffer.begin(), buffer.begin() + pack_size);
			pack_size = 0;
		}
}

public:
	package_codec()
	{
		pack_size = 0;
		transport = nullptr;
	}

	~package_codec()
	{
		//
	}

	void set_transport(reliable_serial *rs)
	{
		rs->set_listener(this);
		transport = rs;
	}

	bool write(const std::vector<uint8_t> &p)
	{
		if(transport == nullptr)
		{
			return false;
		}

		int size = p.size();
		size_t written = 0;
		written += transport->write(&size, sizeof(size));
		written += transport->write(&(p[0]), size);

		return (written == (p.size() + sizeof(size)));
	}
};

class socket_device : public device, public one_to_one_observable<void, const std::vector<uint8_t> &>::listener
{
	package_codec *pc;

	package in;

	typedef int16_t	msgid_t;
	msgid_t msgid = 0;
	//const msgid_t reply_flag = std::numeric_limits<msgid_t>::min();
	const msgid_t reply_flag = 1 << (std::numeric_limits<msgid_t>::digits - 1);

	std::mutex					senders_mutex;
	typedef std::map<msgid_t, std::condition_variable>	sender_condvars_t;
	sender_condvars_t			sender_condvars;
	typedef std::map<msgid_t, bool>	sender_ready_flags_t;
	sender_ready_flags_t			sender_response_received;


	typedef std::map<const package *, msgid_t>		request_msgids_t;
	request_msgids_t							request_msgids;

	msgid_t generate_msgid()
	{
		return msgid++;
	}

	void release_msgid(msgid_t)
	{
		//
	}

public:
	socket_device(reliable_serial *rs) : pc(new package_codec)
	{
		generate_msgid();
		pc->set_listener(this);
		pc->set_transport(rs);
	}

	~socket_device()
	{
		delete pc;
	}

	void process_notification(const std::vector<uint8_t> &p)
	{
		msgid_t msgid = 0;
		memcpy(&msgid, &(p[0]), sizeof(msgid));

		if(p.size() < sizeof(msgid))
		{
			throw("fuck you fucking fuck");
		}
		package *res = new package_t;
		res->resize(p.size() - sizeof(msgid));
		memcpy(&((*res)[0]), &(p[0]) + sizeof(msgid), res->size());
		res->reset_header();

		// highest bit shows if package is a reply
		bool is_reply = msgid & reply_flag;
		// reset highes bit - is it useful?
		msgid &= ~reply_flag;

		if(msgid != 0)
		{
			if(is_reply)
			{
				std::lock_guard<std::mutex> lg(senders_mutex);
				// unlock one who waits
				in = *res;
				sender_response_received[msgid] = true;
				sender_condvars[msgid].notify_one();
				release_msgid(msgid);
			}
			else
			{
				// enque
				request_msgids[res] = msgid;
				notify(res);
			}
		}
		else
		{
			notify(res);
		}
	}

	bool				write				(const package_t &p, msgid_t msgid)
	{
		//package buf;
		std::vector<uint8_t> buf;

		buf.insert(buf.end(), (uint8_t *)(&msgid), (uint8_t *)(&msgid) + 2);
		buf.insert(buf.end(), (uint8_t *)(&(p[0])), (uint8_t *)(&(p[0])) + p.size());

		return pc->write(buf);
	}

	bool				write				(const package_t &p)
	{
		return write(p, 0);
	}

	bool				send				(package_t req, package_t &resp)
	{
		std::unique_lock<decltype(senders_mutex)> lock(senders_mutex);
		msgid_t msgid = generate_msgid();
		bool ok = write(req, msgid);

		if(ok == false)
		{
			release_msgid(msgid);
			return false;
		}

		sender_response_received[msgid] = false;
		while(sender_response_received[msgid] == false)
		{
			sender_condvars[msgid].wait(lock);
		}

		resp = in;

		return true;
	}

	bool				reply				(const package_t *req, const package &reply)
	{
		if(request_msgids.find(req) == request_msgids.end())
		{
			return false;
		}

		msgid_t req_msgid = request_msgids[req];
		req_msgid |= reply_flag;

		std::unique_lock<decltype(senders_mutex)> lock(senders_mutex);

		bool ok = write(reply, req_msgid);

		request_msgids.erase(req);

		if(ok == false)
		{
			return false;
		}

		return true;
	}
};
