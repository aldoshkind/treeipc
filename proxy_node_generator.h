#pragma once

#include <map>

#include "proxy_node_factory_base.h"

namespace treeipc
{

class node_sync;

class proxy_node_generator
{
	node_sync					*cl;

	int						init_factories		();

	typedef std::map<std::string, proxy_node_factory_base *>		property_factories_t;
	property_factories_t											property_factories;

public:
	/*constructor*/			proxy_node_generator			(node_sync *c);
	/*destructor*/			~proxy_node_generator			();

	client_node *generate(std::string type, std::string name);
};

}
