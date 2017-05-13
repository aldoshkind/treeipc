#pragma once

#include "tree/node.h"
#include "device.h"

#include "package.h"

class client : public node, public device::data_listener
{
	void					data				(const device::package_t &p)
	{
		if(p.size() < 1)
		{
			return;
		}

		switch(p.get_cmd())
		{
		default:
		break;
		}
	}

	device					*dev;

	nid_t					nid;

	nid_t					get_rep;

	node					*create				(std::string path)
	{
		return node::generate(path);
	}

	using node::generate;
	node					*generate			()
	{
		client *c = new client;
		c->set_parent(this);
		return c;
	}

public:
	/*constructor*/			client				()
	{
		dev = NULL;
		nid = 0;
	}

	/*destructor*/			~client				()
	{
		//
	}

	void					set_device			(device *d)
	{
		dev = d;
	}

	ls_list_t				ls					() const
	{
		if(dev == NULL)
		{
			return ls_list_t();
		}

		device::package_t req;
		req.set_cmd(CMD_LS);
		req.set_nid(nid);

		device::package_t rep;
		dev->send(req, rep);

		if(rep.get_cmd() != CMD_LS_SUCCESS)
		{
			return ls_list_t();
		}

		ls_list_t res;

		int pos = 0;
		uint32_t count = rep.read<uint32_t>(pos);
		pos += sizeof(uint32_t);
		res.reserve(count);
		for(int i = 0 ; i < count ; i += 1)
		{
			std::string name;
			pos = read_string(rep, name, pos);
			res.push_back(name);
		}

		return res;
	}

	node					*at				(std::string path)
	{
		node *n = node::at(path);
		if(n != NULL)
		{
			return n;
		}

		if(dev == NULL)
		{
			return NULL;
		}

		device::package_t req, rep;
		req.set_cmd(CMD_AT);
		req.set_nid(nid);
		append_string(req, path);

		dev->send(req, rep);

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
};
