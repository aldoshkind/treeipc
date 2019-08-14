#pragma once

#include <map>

#include "qt/acceptor.h"
#include "package_stream.h"
#include "tree/tree_node.h"
#include "server.h"

class conn_server : public acceptor::listener
{
	std::mutex mutex;

	typedef std::shared_ptr<treeipc::server> server_sp;
	typedef std::map<package_stream *, server_sp> clients_t;
	clients_t clients;

	tree_node *root;

public:
	conn_server(tree_node *r)
	{
		root = r;

		printf("%s %p\n", __PRETTY_FUNCTION__, this);
	}

	~conn_server()
	{
		printf("%s %p\n", __PRETTY_FUNCTION__, this);

		mutex.lock();
		printf("lock %p in %s of %p\n", &mutex, __PRETTY_FUNCTION__, this);
#warning Shouldn't we delete package_streams's?
		clients.clear();
		mutex.unlock();
		printf("unlock %p in %s of %p\n", &mutex, __PRETTY_FUNCTION__, this);
	}

	void process_notification(package_stream *s)
	{
		std::lock_guard<std::mutex> lock(mutex);

		server_sp server = std::make_shared<treeipc::server>(s);
		server->set_root(root);
		clients[s] = server;
		s->start();
		printf("%s clients is of size (%d) now\n", __PRETTY_FUNCTION__, (int)clients.size());
	}
};
