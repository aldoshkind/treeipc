#include "client_node.h"

#include "client.h"

/*constructor*/ client_node::client_node(nid_t n)
{
	//dev = NULL;
	nid = n;
}

/*destructor*/ client_node::~client_node()
{
	//
}

/*void					set_device			(device *d)
{
	dev = d;
}*/

void client_node::set_client(client *c)
{
	cl = c;
}

void client_node::set_nid(nid_t nid)
{
	this->nid = nid;
}

/*property_base *client_node::add_property(property_base *p)
{
	cl->request_add_property(this, p);
	return NULL;
}*/

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

#warning до сюда управление не доходит, нужно разобраться

	device::package_t req, rep;
	req.set_cmd(CMD_AT);
	req.set_nid(nid);
	append_string(req, path);

	if(rep.get_cmd() == CMD_AT_ERROR)
	{
		return NULL;
	}

	n = create(path);

	if(n != NULL)
	{
		int pos = 0;
		int prop_count = rep.read<uint16_t>(pos);
		pos += sizeof(uint16_t);

		for(int i = 0 ; i < prop_count ; i += 1)
		{
			std::string type;
			pos = read_string(rep, type, pos);
			std::string name;
			pos = read_string(rep, name, pos);

//			n->add_property(new property_value<double>(name));
		}
	}

	return n;
}

tree_node *client_node::create(std::string path)
{
	return tree_node::generate(path);
}



/*template <class T>
tree_node *client_node_value<T>::generate()
{
	client_node *c = new client_node;
	c->set_parent(this);
	return c;
}*/
