#pragma once

#include "node_sync.h"

namespace treeipc
{

class client : public node_sync
{
public:
	/*constructor*/			client					() {is_server = false;}
	/*destructor*/			~client					() {}
	
	tree_node				*get_root				();
};

}
