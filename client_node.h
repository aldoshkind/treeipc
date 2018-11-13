#pragma once

#include "tree/tree_node_inherited.h"
#include "device.h"
#include "package.h"

class client;

class client_node : public tree_node
{
	nid_t					nid;

	nid_t					get_rep;

	client					*cl;

	//tree_node				*create					(std::string path);

	tree_node				*get					(std::string path, bool create);

public:
	/*constructor*/			client_node				(nid_t n = 0);
	/*destructor*/			~client_node			();

	virtual void			set_client				(client *c);
	virtual void			set_nid					(nid_t nid);

	tree_node::ls_list_t	ls						() const;

	tree_node				*at						(std::string path);
};


