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
		case 'r':
			{
				int pos = 1;
				ls_rep.resize(p.read<uint32_t>(pos));
				pos += sizeof(ls_list_t::size_type);
				for(int i = 0 ; i < ls_rep.size() ; i += 1)
				{
					std::string::size_type st = p.read<uint32_t>(pos);
					pos += sizeof(std::string::size_type);
					std::string str;
					str.resize(st);
					p.read(pos, (void *)str.c_str(), st);
					pos += st;
					ls_rep[i] = str;
				}
			}
		break;
		case 'a':
			{
				int pos = 1;
				std::string::size_type st = p.read<uint32_t>(pos);
				pos += sizeof(std::string::size_type);
				std::string str;
				str.resize(st);
				p.read(pos, (void *)str.c_str(), st);
				generate(str);
				printf("generate %s\n", str.c_str());
			}
		break;
		case 'h':
			{
				int pos = 1;
				get_rep = p.read<uint32_t>(pos);
				pos += sizeof(uint32_t);
				uint32_t prop_count = p.read<uint32_t>(pos);
				pos += sizeof(uint32_t);
				props_t props;
				for(int i = 0 ; i < prop_count ; i += 1)
				{
					std::string type, name;
					read_string(p, type, pos);
					pos += type.size() + sizeof(uint32_t);
					read_string(p, name, pos);
					pos += name.size() + sizeof(uint32_t);
					props.push_back(new property_value<double>(name));
				}
				get_rep_props = props;
			}
		break;
		default:
		break;
		}
	}

	ls_list_t				ls_rep;

	device					*dev;

	typedef uint64_t		nid_t;

	nid_t					get_rep;
	props_t					get_rep_props;

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

		device::package_t pack;
		pack.set_cmd('l');

		dev->write(pack);

		// тут ожидание ответа

		return ls_rep;
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

		device::package_t pack;
		pack.set_cmd('g');
		append_string(pack, path);

		dev->write(pack);

		// ожидание ответа

		if(get_rep == 0)
		{
			return NULL;
		}

		n = create(path);

		if(n != NULL)
		{
			for(auto prop : get_rep_props)
			{
				n->add_property(prop);
			}
		}

		return n;
	}
};
