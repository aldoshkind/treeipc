#pragma once

#include "tree/tree_node_inherited.h".h"
#include "device.h"
#include "package.h"
#include "client.h"

class client_node : public tree_node
{
	nid_t					nid;

	nid_t					get_rep;

	client					*cl;



public:
	/*constructor*/			client_node				(nid_t n = 0);
	/*destructor*/			~client_node			();

	void					set_client				(client *c);
};


template <class T>
class client_node_value : public client_node, public T
{
	tree_node				*create					(std::string path);

	using tree_node::generate;

//	tree_node				*generate				();

	tree_node				*get					(std::string path, bool create);

public:
	client_node_value(nid_t n = 0) : client_node(n)
	{
		//
	}

	tree_node::ls_list_t				ls						() const;

	tree_node				*at						(std::string path);
};






template <class T>
tree_node::ls_list_t client_node_value<T>::ls() const
{
	if(cl == NULL)
	{
		return tree_node::ls_list_t();
	}

	return ls(nid);
}

template <class T>
tree_node *client_node_value<T>::at(std::string path)
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

template <class T>
tree_node *client_node_value<T>::create(std::string path)
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

template <class T>
tree_node *client_node_value<T>::get(std::string path, bool create)
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

	auto cn = tree_node::get_children().operator [](child_id);

	if(cn == NULL)
	{
		return NULL;
	}

	return cn->get(rest_of_path, create);
}
