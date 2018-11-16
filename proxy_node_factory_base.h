#pragma once

#include <string>

namespace treeipc
{

class client_node;

class proxy_node_factory_base
{
public:
	virtual client_node		*generate						(std::string name) = 0;

	virtual /*destructor*/	~proxy_node_factory_base		() {}
};

}
