#include "package_stream.h"

using namespace treeipc;

package_codec::package_codec()
{
	pack_size = 0;
	transport = nullptr;
}

package_codec::~package_codec()
{
	//
}

void package_codec::process_notification(const void *data, size_t size)
{
	//printf("%d bytes read\n", size);
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

void package_codec::set_transport(reliable_bytestream_base *rs)
{
	rs->set_listener(this);
	transport = rs;
}

reliable_bytestream_base *package_codec::get_transport()
{
	return transport;
}

bool package_codec::write(const std::vector<uint8_t> &p)
{
	std::lock_guard<std::recursive_mutex> lock(write_lock);
	//printf("%d bytes write\n", p.size() + 4);
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








package_stream::package_stream(reliable_bytestream_base *rs) : pc(new package_codec)
{
	generate_msgid();
	pc->set_listener(this);
	pc->set_transport(rs);
}

package_stream::~package_stream()
{
	delete pc;
}

package_stream::msgid_t package_stream::generate_msgid()
{
	std::lock_guard<decltype(msgid_released_mutex)> lock(msgid_released_mutex);
	if(msgid_released.size() > 0)
	{
		msgid_t res = msgid_released.back();
		msgid_released.pop_back();
		return res;
	}
	return msgid++;
}

void package_stream::release_msgid(msgid_t msgid)
{
	std::lock_guard<decltype(msgid_released_mutex)> lock(msgid_released_mutex);
	msgid_released.push_back(msgid);
}

void package_stream::process_notification(const std::vector<uint8_t> &p)
{
	msgid_t msgid = 0;
	memcpy(&msgid, &(p[0]), sizeof(msgid));

	if(p.size() < sizeof(msgid))
	{
		throw("p.size() < sizeof(msgid)");
	}
	package *res = new package_t;
	res->resize(p.size() - sizeof(msgid));
	memcpy(&((*res)[0]), &(p[0]) + sizeof(msgid), res->size());
	res->reset_header();

	// highest bit shows if package is a reply
	bool is_reply = msgid & reply_flag;
	// reset highest bit - is it useful?
	msgid &= ~reply_flag;

	if(msgid != 0)
	{
		if(is_reply)
		{
			std::lock_guard<std::mutex> lg(senders_mutex);
			// unlock one who waits
			in = *res;
			delete res;
			sender_response_received[msgid] = true;
			sender_condvars[msgid].notify_one();
		}
		else
		{
			// enque
			std::lock_guard<std::recursive_mutex> lock(request_msgids_mutex);
			request_msgids[res] = msgid;
			notify(res);
		}
	}
	else
	{
		notify(res);
	}
}

bool				package_stream::write				(const package_t &p, msgid_t msgid)
{
	std::vector<uint8_t> buf;

	buf.insert(buf.end(), (uint8_t *)(&msgid), (uint8_t *)(&msgid) + 2);
	buf.insert(buf.end(), (uint8_t *)(&(p[0])), (uint8_t *)(&(p[0])) + p.size());

	return pc->write(buf);
}

bool				package_stream::write				(const package_t &p)
{
	return write(p, 0);
}

bool				package_stream::send				(package_t req, package_t &resp)
{
	std::unique_lock<decltype(senders_mutex)> lock(senders_mutex);
	msgid_t msgid = generate_msgid();
	sender_response_received[msgid] = false;
	bool ok = write(req, msgid);

	if(ok == false)
	{
		release_msgid(msgid);
		return false;
	}

	while(sender_response_received[msgid] == false)
	{
		sender_condvars[msgid].wait(lock);
	}
	release_msgid(msgid);

	resp = in;

	return true;
}

bool				package_stream::reply				(const package_t *req, const package &reply)
{
	std::lock_guard<std::recursive_mutex> request_msgids_lock(request_msgids_mutex);
	if(request_msgids.find(req) == request_msgids.end())
	{
		//printf("unable to reply\n");
		return false;
	}
	
	//printf("reply req %d with %d\n", req->get_nid(), reply.get_nid());

	msgid_t req_msgid = request_msgids[req];
	req_msgid |= reply_flag;

	std::unique_lock<decltype(senders_mutex)> lock(senders_mutex);

	bool ok = write(reply, req_msgid);

	request_msgids.erase(req);
	delete req;

	if(ok == false)
	{
		return false;
	}

	return true;
}

void package_stream::start()
{
	pc->get_transport()->start();
}
