#pragma once

#include "tree/tree_node_inherited.h"
#include "package_stream_base.h"
#include "package.h"

namespace treeipc
{

class node_sync;

class client_node : public tree_node
{
	nid_t					nid;

	nid_t					get_rep;

	node_sync					*cl;

	//tree_node				*create					(std::string path);

	tree_node				*get					(std::string path, bool create);

public:
	/*constructor*/			client_node				(nid_t n = 0);
	/*destructor*/			~client_node			();

	virtual void			set_client				(node_sync *c);
	virtual void			set_nid					(nid_t nid);
	node_sync				*get_client				();

	tree_node::ls_list_t	ls						() const;

	tree_node				*at						(std::string path);

	tree_node				*attach					(std::string name, tree_node *obj, bool append);
	int						remove					(std::string path, bool recursive);
};

}
