#pragma once

#include "client_node.h"

namespace treeipc
{

template <class T>
class client_node_value : public client_node, public T
{
public:
	client_node_value(nid_t n = 0) : client_node(n)
	{
		tree_node::set_type(demangle(typeid(T).name()));
	}

	void set_nid(nid_t nid)
	{
		T::set_nid(nid);
		client_node::set_nid(nid);
	}
};

}
