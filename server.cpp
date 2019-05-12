#include "server.h"

using namespace treeipc;

/*constructor*/ server::server()
{
	current_nid = 0;
}

/*constructor*/ server::server(package_stream_base *d)
{
	set_device(d);
	current_nid = 0;

	set_device(d);
	d->set_listener(this);
}

/*destructor*/ server::~server()
{
	//
}

nid_t server::generate_nid()
{
	return current_nid++;
}

void server::set_root(tree_node *t)
{
	node_sync::set_root(t);
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



/*nid_t server::generate_prid()
{
	return current_prid++;
}*/

/*property_base *server::get_prop(nid_t prid)
{
	props_t::iterator it = props.find(prid);
	return (it == props.end()) ? NULL : it->second;
}*/
