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
	//
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

	typename tree_node::children_t::size_type child_id = tree_node::find(name);
	if(child_id == std::numeric_limits<typename tree_node::children_t::size_type>::max())
	{
#warning передать create в fetch
		tree_node *n = cl->fetch_node(nid, name);
		if(n != NULL)
		{
			child_id = tree_node::insert(name, n);
		}
		else
		{
			return NULL;
		}
	}

	auto cn = tree_node::get_children()[child_id];

	if(cn == NULL)
	{
		return NULL;
	}

	return cn->get(rest_of_path, create);
}

tree_node::ls_list_t client_node::ls() const
{
	if(cl == NULL)
	{
		return tree_node::ls_list_t();
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
	if(cl != nullptr)
	{
		auto res = tree_node::attach(name, obj, append);
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
	printf("client node remove %p\n", res);
	return res;
}

node_sync *client_node::get_client()
{
	return cl;
}
