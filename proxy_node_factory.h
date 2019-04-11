#pragma once

#include "client_node_value.h"
#include "property_value_fake.h"
#include "proxy_node_factory_base.h"

namespace treeipc
{

template <class T>
class proxy_node_factory : public proxy_node_factory_base
{
	node_sync					*cl;

public:
	/*constructor*/			proxy_node_factory	(node_sync *c) : cl(c)
	{
		//
	}

	/*destructor*/			~proxy_node_factory	()
	{
		//
	}

	client_node				*generate			(std::string /*name*/)
	{
		auto node = new client_node_value<property_value_fake<T>>();
		node->property_value_fake<T>::set_client(cl);
		node->client_node::set_client(cl);
		return node;
	}
};

}
