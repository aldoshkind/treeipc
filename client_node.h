#pragma once

#include "tree/node.h"
#include "device.h"

class client;

class client_node : public node
{
	nid_t					nid;

	nid_t					get_rep;

	client					*cl;

	node					*create					(std::string path);

	using node::generate;

	node					*generate				();

	node					*get					(std::string path, bool create);

public:
	/*constructor*/			client_node				(nid_t n = 0);
	/*destructor*/			~client_node			();

	ls_list_t				ls						() const;

	node					*at						(std::string path);

	void					set_client				(client *c);
};
