#pragma once

#include <map>

#include "tree/node.h"
#include "device.h"

#include "package.h"

class tracker : public node::listener_t, resource::new_property_listener, public property_listener
{
	device						*dev;
	nid_t						nid;
	node						*target;

public:
	/*constructor*/				tracker				(node *n)
	{
		target = n;
	}

	/*destructor*/				~tracker			()
	{
		//
	}

	void						set_device			(device *d)
	{
		dev = d;
	}

	void						set_nid				(nid_t n)
	{
		nid = n;
	}

	node						*get_node			() const
	{
		return target;
	}

	void						process				(const device::package_t &p)
	{
		if(p.size() < 1)
		{
			return;
		}

		switch(p.get_cmd())
		{
		case 'l':
			if(dev != NULL)
			{
				device::package_t pack;
				pack.set_cmd('r');
				node::ls_list_t items = target->ls();

				pack.append<uint32_t>(items.size());

				for(auto item : items)
				{
					append_string(pack, item);
				}
				dev->write(pack);
			}
		break;
		case 'g':
			if(dev != NULL)
			{
				device::package_t pack;
				pack.set_cmd('h');
				std::string path;
				read_string(p, path, 1);
				node *n = target->at(path);

				static uint32_t id = 1;

				if(n == NULL)
				{
					pack.append<uint32_t>(0);
				}
				else
				{
					/*nodes_by_id[id] = n;
					ids_by_node[n] = id;
					append(pack, id);

					props_t props = n->get_properties();

					append<uint32_t>(pack, props.size());
					for(auto prop : props)
					{
						append_string(pack, prop->get_type());
						append_string(pack, prop->get_name());
					}*/
				}

				id += 1;
				dev->write(pack);
			}
		break;
		default:
		break;
		}
	}
};

class server : public device::data_listener
{
	typedef std::map<nid_t, tracker *>		trackers_t;
	trackers_t								trackers;

	tracker					*get_tracker	(nid_t nid)
	{
		trackers_t::iterator it = trackers.find(nid);
		return (it == trackers.end()) ? NULL : it->second;
	}

	tracker					*get_tracker	(node *n)
	{
		for(trackers_t::iterator it = trackers.begin() ; it != trackers.end() ; ++it)
		{
			if(it->second->get_node() == n)
			{
				return it->second;
			}
		}
	}

	void					cmd_at			(const device::package_t &p)
	{
		nid_t nid = p.get_nid();
		tracker *t = get_tracker(nid);
		device::package_t resp;
		if(t == NULL)
		{
			resp.set_cmd(CMD_AT_ERROR);
			resp.set_msgid(p.get_msgid());
			resp.set_nid(p.get_nid());
		}
		else
		{
			node *n = t->at(p);
			tracker *tt = get_tracker(n); // tt = target tracker
			if()
		}
	}

	void					data				(const device::package_t &p)
	{
		switch(p.get_cmd())
		{
		case CMD_AT:
			cmd_at(p);

		break;
		default:
		break;
		}
	}

	void					child_added			(node *n)
	{
		printf("child added %s\n", n->get_name().c_str());

		if(dev != NULL)
		{
			device::package_t pack;
			pack.set_cmd('a');

			int pos = 0;

			std::string item;
			item = n->get_name();
			pos = pack.size();

			append_string(pack, item);

			dev->write(pack);
		}
	}

	void					child_removed		(node *, std::string name)
	{
		//
	}

	device					*dev;
	node					*target;
	nid_t					current_nid;

	nid_t					generate_nid		()
	{
		return current_nid++;
	}

	void					do_track			(node *n)
	{
		tracker *tr = new tracker(t);
		tr->set_nid(generate_nid());
		trackers[nid] = t;
	}

	void					untrack				(node */*n*/)
	{
		//
	}

public:
	/*constructor*/			server				()
	{
		dev = NULL;
		target = NULL;
		current_nid = 1;
	}

	/*destructor*/			~server				()
	{
		//
	}

	void					set_target			(node *t)
	{		
		target = t;
		if(t != NULL)
		{
			do_track(t);
		}
		else
		{
			untrack(t);
			// замочить трекер с nid == 0 ?
		}
	}

	void					set_device			(device *d)
	{
		dev = d;
	}
};
