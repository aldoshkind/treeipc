#include "server.h"

/*constructor*/ server::server()
{
	dev = NULL;
	target = NULL;
	current_nid = 0;
//	current_prid = 0;
}

/*constructor*/ server::server(device *d)
{
	dev = NULL;
	target = NULL;
	current_nid = 0;
//	current_prid = 0;

	set_device(d);
}

/*destructor*/ server::~server()
{
	//
}

void server::child_added(tree_node *n)
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

void server::child_removed(tree_node *, std::string name)
{
	//
}

nid_t server::generate_nid()
{
	return current_nid++;
}

nid_t server::do_track(tree_node *n)
{
	nid_t nid = generate_nid();
	tracked[nid] = n;
	return nid;
}

void server::untrack(tree_node */*n*/)
{
	//
}

void server::set_target(tree_node *t)
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
	tree_node *t = get_node(nid);
	device::package_t resp;
	if(t == NULL)
	{
		resp.set_cmd(CMD_LS_ERROR);
		resp.set_nid(p.get_nid());
	}
	else
	{
		tree_node::ls_list_t list = t->ls();

		resp.set_cmd(CMD_LS_SUCCESS);
		resp.set_nid(nid);

		resp.append<uint32_t>(list.size());
		for(auto item : list)
		{
			append_string(resp, item);
		}
	}
	dev->reply(p, resp);
}

void server::process_notification(const device::package_t &p)
{
	switch(p.get_cmd())
	{
	case CMD_AT:
		cmd_at(p);
	break;
	case CMD_LS:
		cmd_ls(p);
	break;
	case CMD_PROP_GET_VALUE:
		cmd_get_prop(p);
	break;
	case CMD_SUBSCRIBE:
		cmd_subscribe(p, false);
	break;
	case CMD_UNSUBSCRIBE:
		cmd_subscribe(p, true);
	break;
	case CMD_PROP_SET_VALUE:
		cmd_prop_value(p);
	break;
	default:
	break;
	}
}

void server::cmd_at(const device::package_t &p)
{
	nid_t nid = p.get_nid();
	tree_node *t = get_node(nid);
	tree_node *n = NULL;
	device::package_t resp;
	if(t == NULL)
	{
		resp.set_cmd(CMD_AT_ERROR);
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
			resp.set_nid(p.get_nid());
		}
		else
		{
			nid_t nid = get_nid(n);
			resp.set_cmd(CMD_AT_SUCCESS);
			resp.set_nid(nid);

			auto prop = dynamic_cast<property_base *>(n);
			if(prop != nullptr)
			{
				append_string(resp, prop->get_type());
			}
			else
			{
				append_string(resp, "");
			}
		}
	}
	dev->reply(p, resp);

	if(n != NULL)
	{
		n->add_listener(this);
	}
}

tree_node *server::get_node(nid_t nid)
{
	tracked_t::iterator it = tracked.find(nid);
	return (it == tracked.end()) ? NULL : it->second;
}

bool server::get_nid(tree_node *n, nid_t &nid, bool track)
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

/*void server::new_property(resource *r, property_base *prop)
{
	tree_node_t *n = dynamic_cast<tree_node_t *>(r);

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

	nid_t prid;
	get_nid(prop, prid);

	p.append(prid);
	append_string(p, type);
	append_string(p, prop->get_name());

	dev->write(p);
}*/

nid_t server::get_nid(tree_node *n)
{
	nid_t nid;
	get_nid(n, nid, true);
	return nid;
}

bool server::get_nid(property_base *p, nid_t &nid)
{
	auto n = dynamic_cast<tree_node *>(p);
	if(n == nullptr)
	{
		return false;
	}
	return get_nid(n, nid, false);


	/*for(props_t::iterator it = props.begin() ; it != props.end() ; ++it)
	{
		if(it->second == p)
		{
			prid = it->first;
			return true;
		}
	}

	prid = generate_prid();
	props[prid] = p;

	return true;*/

	return false;
}

/*nid_t server::generate_prid()
{
	return current_prid++;
}*/

void server::cmd_get_prop(const device::package_t &p)
{
	nid_t nid = p.get_nid();
	property_base *prop = get_prop(nid);

	device::package_t resp;
	if(prop == NULL)
	{
		resp.set_cmd(CMD_PROP_GET_VALUE_ERROR);
		resp.set_nid(p.get_nid());
	}
	else
	{
		serializer_base::buffer_t buf = serializer.serialize(prop);

		resp.set_cmd(CMD_PROP_GET_VALUE_SUCCESS);
		resp.set_nid(nid);

		resp.append<uint32_t>(buf.size());
		resp.append(&(buf[0]), buf.size());
	}
	dev->reply(p, resp);
}

/*property_base *server::get_prop(nid_t prid)
{
	props_t::iterator it = props.find(prid);
	return (it == props.end()) ? NULL : it->second;
}*/

property_base *server::get_prop(nid_t nid)
{
	tracked_t::iterator it = tracked.find(nid);
	if(it == tracked.end())
	{
		return nullptr;
	}

	property_base *pb = dynamic_cast<property_base *>(it->second);

	return pb;
}

void server::cmd_subscribe(const device::package_t &p, bool erase)
{
	const nid_t &nid = p.get_nid();

	auto *pb = get_prop(nid);
	if(pb == nullptr)
	{
		return;
	}

	if(erase == false)
	{
		//props_subscribed.insert(nid);
		pb->add_listener(this);
	}
	else
	{
		//props_subscribed.erase(nid);
		pb->remove_listener(this);
	}
}

void server::updated(property_base *prop)
{
	nid_t nid;
	get_nid(prop, nid);
	/*if(props_subscribed.find(prid) == props_subscribed.end())
	{
		return;
	}*/

	device::package_t resp;
	serializer_base::buffer_t buf = serializer.serialize(prop);

	resp.set_cmd(CMD_PROP_VALUE_UPDATED);
	resp.set_nid(nid);

	resp.append<uint32_t>(buf.size());
	resp.append(&(buf[0]), buf.size());

	dev->write(resp);
}

void server::cmd_prop_value(const device::package_t &p)
{
	nid_t nid = p.get_nid();

	printf("value for %d\n", nid);

	property_base *pb = dynamic_cast<property_base *>(tracked[nid]);

	if(pb == nullptr)
	{
		printf("%s: node with prid %d not found\n", __PRETTY_FUNCTION__, nid);
		return;
	}

	serializer_base::buffer_t buf;
	buf.resize(p.data_size());
	p.read(0, &(buf[0]), buf.size());

	serializer.deserialize(buf, pb);
}
