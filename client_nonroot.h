#pragma once

#include "node_sync.h"

namespace treeipc
{

class client_nonroot : public node_sync
{
public:
	/*constructor*/			client_nonroot					() {is_server = false;}
	/*destructor*/			~client_nonroot					() {}
	
	void					set_root				(tree_node *);
	
private:
	void					child_added				(tree_node *p, tree_node *n);
};

}
