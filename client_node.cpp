#include "client_node.h"

#include "client.h"

using namespace treeipc;

/*constructor*/ client_node::client_node(nid_t n)
{
	//dev = NULL;
	nid = n;
}

/*destructor*/ client_node::~client_node()
{
	if(cl != nullptr)
	{
		cl->remove_client_node(nid);
	}
}

void client_node::set_client(node_sync *c)
{
	cl = c;
}

void client_node::set_nid(nid_t nid)
{
	this->nid = nid;
}

tree_node *client_node::get(std::string path, bool create)
{
	if(cl == NULL)
	{
		return NULL;
	}

	std::string::size_type begin = path.find_first_not_of('/');
	if((begin != 0) && (begin != std::string::npos))
	{
		path = path.substr(begin);
	}

	if(path == "/" || path.size() == 0)
	{
		return dynamic_cast<tree_node *>(this);
	}

	std::string name, rest_of_path;
	extract_next_level_name(path, name, rest_of_path);

	if(children_map.find(name) == children_map.end())
	{
		if(create == false)
		{
			return nullptr;
		}
		tree_node *n = cl->fetch_node(nid, name);
		if(n != NULL)
		{
			bool ok = tree_node::insert(name, n, true);
			if(ok == false)
			{
				return nullptr;
			}
		}
		else
		{
			return nullptr;
		}
	}

	auto cn = tree_node::get_children()[name];

	if(cn == nullptr)
	{
		return nullptr;
	}

	return cn->get(rest_of_path, create);
}

tree_node::string_list_t client_node::ls() const
{
	if(cl == NULL)
	{
		return tree_node::string_list_t();
	}

	return cl->ls(nid);
}

tree_node *client_node::at(std::string path)
{
	tree_node *n = tree_node::at(path);
	if(n != NULL)
	{
		return n;
	}

	return n;
}

tree_node *client_node::attach(std::string name, tree_node *obj, bool append)
{
	if(obj == nullptr)
	{
		return nullptr;
	}
	
	auto children = tree_node::get_children();
	if(children.find(name) != children.end())
	{
		printf("%s: %s already has %s as child\n", __func__, get_name().c_str(), obj->get_name().c_str());
		return obj;
	}
	
	if(cl != nullptr)
	{
		auto res = tree_node::attach(name, obj, append);
		
		
		client_node *obj_as_client_node = dynamic_cast<client_node *>(obj);
		if(obj_as_client_node != nullptr && obj_as_client_node->get_client() == get_client())
		{
			return res;
		}
		
		bool ok = cl->attach(nid, name, obj);
		if(!ok)
		{
			tree_node::remove(name);
			printf("failed to attach client node %p\n", res);
			return res;
		}
	}

	return nullptr;
}

int client_node::remove(std::string path, bool recursive)
{
	int res = tree_node::remove(path, recursive);
	printf("client node remove %d\n", res);
	return res;
}

node_sync *client_node::get_client()
{
	return cl;
}

void client_node::subscribe_add_remove()
{
	if(cl != nullptr)
	{
		cl->subscribe_add_remove(nid);
	}
}
