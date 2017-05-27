#include "client.h"

/*constructor*/ client::client() : generator(this)
{
	dev = NULL;
	root_node = NULL;
}

/*destructor*/ client::~client()
{
	//
}

void client::set_device(device *d)
{
	dev = d;
}

client_node *client::get_root()
{
	if(root_node != NULL)
	{
		return root_node;
	}
	root_node = fetch_node(0, "/");
	return root_node;
}

client_node *client::fetch_node(nid_t nid, std::string name)
{
	device::package_t req, rep;
	req.set_cmd(CMD_AT);
	req.set_nid(nid);
	append_string(req, name);

	dev->send(req, rep);

	if(rep.get_cmd() == CMD_AT_ERROR)
	{
		return NULL;
	}

	return tracked[rep.get_nid()];
}

void client::data(const device::package_t &p)
{
	if(p.size() < 1)
	{
		return;
	}

	switch(p.get_cmd())
	{
	case CMD_AT_SUCCESS:
	{
		client_node *n = new client_node(p.get_nid());
		n->set_client(this);

		tracked[p.get_nid()] = n;
	}
	break;
	case CMD_NEW_PROP:
		process_new_property(p);
	break;
	case CMD_PROP_VALUE:
		process_prop_value(p);
	break;
	default:
	break;
	}
}

void client::process_new_property(const device::package_t &p)
{
	tracked_t::iterator it = tracked.find(p.get_nid());
	if(it == tracked.end())
	{
		return;
	}

	std::string type;
	std::string name;

	int pos = 0;
	prid_t prid = p.read<prid_t>(pos);
	pos += sizeof(prid);
	pos = read_string(p, type, pos);
	read_string(p, name, pos);
	printf("new prop %s %s\n", type.c_str(), name.c_str());

	node *n = it->second;

	property_base *prop = generate_property(type, name);
	property_fake *pf = dynamic_cast<property_fake *>(prop);
	if(pf != NULL)
	{
		pf->set_prid(prid);
	}

	props[prid] = prop;

	if(prop == NULL)
	{
		// is this behaviour correct?
		return;
	}

	n->add_property(prop);
}

property_base *client::generate_property(std::string type, std::string name)
{
	return generator.generate(type, name);
}

client_node::ls_list_t client::ls(nid_t nid)
{
	device::package_t req;
	req.set_cmd(CMD_LS);
	req.set_nid(nid);

	device::package_t rep;
	dev->send(req, rep);

	if(rep.get_cmd() != CMD_LS_SUCCESS)
	{
		return client_node::ls_list_t();
	}

	client_node::ls_list_t res;

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

int property_generator::init_property_factories()
{
	property_factories["std::string"] = new fake_property_factory<std::string>(cl);
	property_factories[typeid(double).name()] = new fake_property_factory<double>(cl);
	property_factories[typeid(int).name()] = new fake_property_factory<int>(cl);

	return 0;
}


/*constructor*/ property_generator::property_generator(client *c) : cl(c)
{
	init_property_factories();
}

/*destructor*/ property_generator::~property_generator()
{
	//
}

property_base *property_generator::generate(std::string type, std::string name)
{
	property_factories_t::iterator it = property_factories.find(type);
	if(it == property_factories.end())
	{
		return NULL;
	}

	return it->second->generate(name);
}

void client::update_prop(prid_t prid)
{
	device::package_t req, rep;

	req.set_prid(prid);
	req.set_cmd(CMD_PROP_UPDATE);

	dev->send(req, rep);

	props_t::iterator it = props.find(prid);
	if(it == props.end())
	{
		return;
	}

	uint32_t sz = rep.read<uint32_t>(0);
	serializer_base::buffer_t buf;
	buf.resize(sz);
	rep.read(sizeof(sz), &(buf[0]), sz);

	property_fake *pvf = dynamic_cast<property_fake *>(it->second);
	if(pvf != NULL)
	{
		pvf->set_deserialization(true);
	}

	serializer.deserialize(buf, it->second);

	if(pvf != NULL)
	{
		pvf->set_deserialization(false);
	}

	return;
}

void client::subscribe(prid_t prid)
{
	device::package_t req;

	req.set_prid(prid);
	req.set_cmd(CMD_SUBSCRIBE);

	dev->write(req);
}

void client::unsubscribe(prid_t prid)
{
	device::package_t req;

	req.set_prid(prid);
	req.set_cmd(CMD_UNSUBSCRIBE);

	dev->write(req);
}

void property_fake::update_value() const
{
	cl->update_prop(get_prid());
}

void property_fake::subscribe() const
{
	cl->subscribe(get_prid());
}

void property_fake::unsubscribe() const
{
	cl->unsubscribe(get_prid());
}

void client::process_prop_value(const device::package_t &p)
{
	const prid_t &prid = p.get_prid();
	props_t::iterator it = props.find(prid);
	if(it == props.end())
	{
		return;
	}

	uint32_t sz = p.read<uint32_t>(0);
	serializer_base::buffer_t buf;
	buf.resize(sz);
	p.read(sizeof(sz), &(buf[0]), sz);

	property_fake *pvf = dynamic_cast<property_fake *>(it->second);
	if(pvf != NULL)
	{
		pvf->set_deserialization(true);
	}

	serializer.deserialize(buf, it->second);

	if(pvf != NULL)
	{
		pvf->set_deserialization(false);
	}
	it->second->notify_change();
}
