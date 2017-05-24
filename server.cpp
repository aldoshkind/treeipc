#include "server.h"

/*constructor*/ server::server()
{
	dev = NULL;
	target = NULL;
	current_nid = 0;
}

/*destructor*/ server::~server()
{
	//
}

void server::child_added(node *n)
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

void server::child_removed(node *, std::string name)
{
	//
}

nid_t server::generate_nid()
{
	return current_nid++;
}

nid_t server::do_track(node *n)
{
	nid_t nid = generate_nid();
	tracked[nid] = n;
	return nid;
}

void server::untrack(node */*n*/)
{
	//
}

void server::set_target(node *t)
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

void server::set_device(device *d)
{
	dev = d;
}

void server::cmd_ls(const device::package_t &p)
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

void server::data(const device::package_t &p)
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

void server::cmd_at(const device::package_t &p)
{
	nid_t nid = p.get_nid();
	node *t = get_node(nid);
	node *n = NULL;
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

		n = t->at(path);

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
	}
	dev->write(resp);

	if(n != NULL)
	{
		n->resource::add_listener(this);
	}
}

node *server::get_node(nid_t nid)
{
	tracked_t::iterator it = tracked.find(nid);
	return (it == tracked.end()) ? NULL : it->second;
}

bool server::get_nid(node *n, nid_t &nid, bool track)
{
	for(tracked_t::iterator it = tracked.begin() ; it != tracked.end() ; ++it)
	{
		if(it->second == n)
		{
			nid = it->first;
			return true;
		}
	}

	if(track == true)
	{
		nid = do_track(n);
		return true;
	}
	return false;
}

void server::new_property(resource *r, property_base *prop)
{
	node *n = dynamic_cast<node *>(r);

	if(n == NULL)
	{
		return;
	}

	nid_t nid;

	if(get_nid(n, nid, false) == false)
	{
		return;
	}

	device::package_t p;
	p.set_cmd(CMD_NEW_PROP);
	p.set_nid(nid);

	std::string type = prop->get_type();
	if(type == typeid(std::string).name())
	{
		type = "std::string";
	}

	append_string(p, type);
	append_string(p, prop->get_name());

	dev->write(p);
}

nid_t server::get_nid(node *n)
{
	nid_t nid;
	get_nid(n, nid, true);
	return nid;
}
