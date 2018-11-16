#pragma once

#include <map>
#include <set>

#include "node_sync.h"

namespace treeipc
{

class server : public node_sync
{
	nid_t					current_nid;

	nid_t					generate_nid		();

public:
	/*constructor*/			server				();
	/*constructor*/			server				(device *d);
	/*destructor*/			~server				();
	
	void					set_root			(tree_node *root);
};

}
