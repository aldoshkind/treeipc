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

client_node::ls_list_t client_node::ls() const
{
	if(cl == NULL)
	{
		return ls_list_t();
	}

	return cl->ls(nid);
}

node *client_node::at(std::string path)
{
	node *n = node::at(path);
	if(n != NULL)
	{
		return n;
	}

/*		if(dev == NULL)
	{
		return NULL;
	}*/

	device::package_t req, rep;
	req.set_cmd(CMD_AT);
	req.set_nid(nid);
	append_string(req, path);

//		dev->send(req, rep);

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

			n->add_property(new property_value<double>(name));
		}
	}

	return n;
}

node *client_node::create(std::string path)
{
	return node::generate(path);
}

void client_node::set_client(client *c)
{
	cl = c;
}

node *client_node::generate()
{
	client_node *c = new client_node;
	c->set_parent(this);
	return c;
}

node *client_node::get(std::string path, bool create)
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
		return dynamic_cast<node *>(this);
	}

	std::string name, rest_of_path;
	extract_next_level_name(path, name, rest_of_path);

	typename children_t::size_type child_id = find(name);
	if(child_id == std::numeric_limits<typename children_t::size_type>::max())
	{
#warning передать create в fetch
		node *n = cl->fetch_node(nid, name);
		if(n != NULL)
		{
			child_id = insert(name, n);
		}
		else
		{
			return NULL;
		}
	}

	client_node *cn = dynamic_cast<client_node *>(get_children().operator [](child_id));

	if(cn == NULL)
	{
		return NULL;
	}

	return cn->get(rest_of_path, create);
}
