#pragma once

#include "acceptor.h"
#include "socket_device.h"
#include "tree/tree_node.h"
#include "server.h"

class conn_server : public acceptor::listener
{
	std::mutex mutex;

	object_status::sp state;

	class client
	{
		socket_client sc;
		socket_device sd;
		treeipc::server srv;

	public:
		client(socket_sp socket, tree_node *n) : sc(socket), sd(&sc), srv(&sd)
		{
			srv.set_root(n);
			sd.set_listener(&srv);
		}
	};

	typedef std::shared_ptr<client> client_sp;
	typedef std::map<socket_sp::element_type *, client_sp> clients_t;
	clients_t clients;

	tree_node *root;

public:
	conn_server(tree_node *r) : state(new object_status)
	{
		root = r;

		printf("%s %p\n", __PRETTY_FUNCTION__, this);
	}

	~conn_server()
	{
		state->mut.lock();
		state->set_ok(false);
		state->mut.unlock();
		printf("%s %p\n", __PRETTY_FUNCTION__, this);

		mutex.lock();
		printf("lock %p in %s of %p\n", &mutex, __PRETTY_FUNCTION__, this);
		clients.clear();
		mutex.unlock();
		printf("unlock %p in %s of %p\n", &mutex, __PRETTY_FUNCTION__, this);
	}

	void process_notification(socket_sp s)
	{
		std::lock_guard<std::mutex> lock(mutex);

		client_sp cl(new client(s, root));
		clients[s.get()] = cl;

		printf("%s clients is of size (%d) now\n", __PRETTY_FUNCTION__, (int)clients.size());
		/*s->begin_async_read();
		s->on_ready = boost::bind(&conn_server::ready, this, _1, state);
		s->on_eof = boost::bind(&conn_server::disconnect, this, _1, state);*/
	}

	/*void disconnect(socket_sp s, object_status::sp i)
	{
		std::lock_guard<decltype(i->mut)>(i->mut);
		if(i->is_ok() != true)
		{
			return;
		}

		printf("%s %p\n", __PRETTY_FUNCTION__, this);
		mutex.lock();
		printf("lock %p in %s of %p\n", &mutex, __PRETTY_FUNCTION__, this);
		clients.erase(s.get());
		mutex.unlock();
		printf("unlock %p in %s of %p\n", &mutex, __PRETTY_FUNCTION__, this);
	}*/
};
