#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>

#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <map>

#include "package_stream_base.h"
#include "reliable_bytestream_base.h"

namespace treeipc
{

class package_codec : public one_to_one_observable<void, const std::vector<uint8_t> &>, public reliable_bytestream_base::listener
{	
public:
	package_codec();
	~package_codec();

	void set_transport(reliable_bytestream_base *rs);
	reliable_bytestream_base *get_transport();
	bool write(const std::vector<uint8_t> &p);
	
private:
	reliable_bytestream_base *transport;

	//package buffer;
	std::vector<uint8_t> buffer;
	typedef uint32_t pack_size_t;
	pack_size_t pack_size;
	std::recursive_mutex write_lock;

	void process_notification(const void *data, size_t size);
};





class package_stream : public package_stream_base, public one_to_one_observable<void, const std::vector<uint8_t> &>::listener
{
private:
	typedef uint16_t msgid_t;
	
public:
	package_stream(reliable_bytestream_base *rs);
	~package_stream();

	void process_notification(const std::vector<uint8_t> &p);
	bool write(const package_t &p, msgid_t msgid);
	bool write(const package_t &p);
	bool send(package_t req, package_t &resp);
	bool reply(const package_t *req, const package &reply);
	void start();
	
private:
	package_codec *pc;

	package in;
	msgid_t msgid = 0;
	const msgid_t reply_flag = 1 << (sizeof(msgid_t) * CHAR_BIT - 1);

	std::mutex					senders_mutex;
	typedef std::map<msgid_t, std::condition_variable>	sender_condvars_t;
	sender_condvars_t			sender_condvars;
	typedef std::map<msgid_t, bool>	sender_ready_flags_t;
	sender_ready_flags_t			sender_response_received;


	typedef std::map<const package *, msgid_t>		request_msgids_t;
	request_msgids_t								request_msgids;
	std::recursive_mutex							request_msgids_mutex;
	
	typedef std::vector<msgid_t> msgid_released_t;
	msgid_released_t msgid_released;
	std::recursive_mutex msgid_released_mutex;

	msgid_t generate_msgid();
	void release_msgid(msgid_t msgid);
};

}
