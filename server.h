#pragma once

#include <map>

#include "tree/node.h"
#include "device.h"

#include "package.h"

class server : public device::data_listener
{
	typedef std::map<nid_t, node *>		tracked_t;
	tracked_t							tracked;

	node					*get_node	(nid_t nid)
	{
		tracked_t::iterator it = tracked.find(nid);
		return (it == tracked.end()) ? NULL : it->second;
	}

	nid_t					get_nid	(node *n)
	{
		for(tracked_t::iterator it = tracked.begin() ; it != tracked.end() ; ++it)
		{
			if(it->second == n)
			{
				return it->first;
			}
		}
		return do_track(n);
	}

	void					cmd_at			(const device::package_t &p)
	{
		nid_t nid = p.get_nid();
		node *t = get_node(nid);
		device::package_t resp;
		if(t == NULL)
		{
			resp.set_cmd(CMD_AT_ERROR);
			resp.set_msgid(p.get_msgid());
			resp.set_nid(p.get_nid());
		}
		else
		{
			std::string path;
			read_string(p, path);

			node *n = t->at(path);

			if(n == NULL)
			{
				resp.set_cmd(CMD_AT_ERROR);
				resp.set_msgid(p.get_msgid());
				resp.set_nid(p.get_nid());
			}

			nid_t nid = get_nid(n); // tt = target tracker
			resp.set_cmd(CMD_AT_SUCCESS);
			resp.set_msgid(p.get_msgid());
			resp.set_nid(nid);

			node::props_t props = n->get_properties();
			resp.append<uint16_t>(props.size());
			for(auto prop : props)
			{
				append_string(resp, prop->get_type());
				append_string(resp, prop->get_name());
			}
		}
		dev->write(resp);
	}

	void					cmd_ls				(const device::package_t &p)
	{
		nid_t nid = p.get_nid();
		node *t = get_node(nid);
		device::package_t resp;
		if(t == NULL)
		{
			resp.set_cmd(CMD_LS_ERROR);
			resp.set_msgid(p.get_msgid());
			resp.set_nid(p.get_nid());
		}
		else
		{
			node::ls_list_t list = t->ls();

			resp.set_cmd(CMD_LS_SUCCESS);
			resp.set_msgid(p.get_msgid());
			resp.set_nid(nid);

			resp.append<uint32_t>(list.size());
			for(auto item : list)
			{
				append_string(resp, item);
			}
		}
		dev->write(resp);
	}

	void					data				(const device::package_t &p)
	{
		switch(p.get_cmd())
		{
		case CMD_AT:
			cmd_at(p);
		break;
		case CMD_LS:
			cmd_ls(p);
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

	nid_t					do_track			(node *n)
	{
		nid_t nid = generate_nid();
		tracked[nid] = n;
		return nid;
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
		current_nid = 0;
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
