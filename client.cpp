#include "client.h"

#include <QString>

#include "client_node.h"

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

	std::lock_guard<std::mutex> lg(tracked_mutex);
	dev->send(req, rep);

	if(rep.get_cmd() == CMD_AT_ERROR)
	{
		return NULL;
	}

	std::string type;
	read_string(rep, type);

	auto cl = generator.generate(type, name);

	if(cl == nullptr)
	{
		return nullptr;
	}

	cl->set_nid(rep.get_nid());
	tracked[rep.get_nid()] = cl;
	cl->set_client(this);

	return tracked[rep.get_nid()];
}

void client::process_notification(const device::package_t &p)
{
	if(p.size() < 1)
	{
		return;
	}

	switch(p.get_cmd())
	{
	case CMD_AT_SUCCESS:
	{
		//printf("CMD_AT_SUCCESS\n");
		client_node *n = new client_node(p.get_nid());
		n->set_client(this);

		tracked[p.get_nid()] = n;
		//printf("CMD_AT_SUCCESS 0\n");
	}
	break;
	//case CMD_NEW_PROP:
		//printf("CMD_NEW_PROP\n");
		//process_new_property(p);
		//printf("CMD_NEW_PROP 0\n");
	break;
	case CMD_PROP_VALUE_UPDATED:
		process_prop_value(p);
	break;
	default:
	break;
	}
}

/*void client::process_new_property(const device::package_t &p)
{
	std::lock_guard<std::mutex> lg(tracked_mutex);
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

	tree_node_t *n = it->second;

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

	n->node::add_property(prop);
}*/

/*property_base *client::generate_property(std::string type, std::string name)
{
	return generator.generate(type, name);
}*/

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

class untyped_node_factory : public proxy_node_factory_base
{
	client *cl;
public:
	untyped_node_factory(client *c) : cl(c)
	{
		//
	}

	client_node		*generate			(std::string name)
	{
		auto n = new client_node();
		n->set_client(cl);
		return n;
	}
};

int proxy_node_generator::init_factories()
{
	property_factories[""] = new untyped_node_factory(cl);
	property_factories[typeid(double).name()] = new proxy_node_factory<double>(cl);
	property_factories[typeid(int).name()] = new proxy_node_factory<int>(cl);
	property_factories[typeid(QString).name()] = new proxy_node_factory<QString>(cl);

	return 0;
}


/*constructor*/ proxy_node_generator::proxy_node_generator(client *c) : cl(c)
{
	init_factories();
}

/*destructor*/ proxy_node_generator::~proxy_node_generator()
{
	//
}

client_node *proxy_node_generator::generate(std::string type, std::string name)
{
	property_factories_t::iterator it = property_factories.find(type);
	if(it == property_factories.end())
	{
		return NULL;
	}

	return it->second->generate(name);
}

void client::update_prop(nid_t nid)
{
	device::package_t req, rep;

	req.set_nid(nid);
	req.set_cmd(CMD_PROP_GET_VALUE);

	dev->send(req, rep);

	/*props_t::iterator it = props.find(prid);
	if(it == props.end())
	{
		return;
	}*/

	tracked_t::iterator it = tracked.find(nid);
	if(it == tracked.end())
	{
		return;
	}

	auto prop = dynamic_cast<property_base *>(it->second);

	if(prop == nullptr)
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

	serializer.deserialize(buf, prop);

	if(pvf != NULL)
	{
		pvf->set_deserialization(false);
	}

	return;
}

void client::subscribe(nid_t nid)
{
	device::package_t req;

	req.set_nid(nid);
	req.set_cmd(CMD_SUBSCRIBE);

	dev->write(req);
}

void client::unsubscribe(nid_t nid)
{
	device::package_t req;

	req.set_nid(nid);
	req.set_cmd(CMD_UNSUBSCRIBE);

	dev->write(req);
}

void property_fake::update_value() const
{
	cl->update_prop(get_nid());
}

void property_fake::subscribe() const
{
	cl->subscribe(get_nid());
}

void property_fake::unsubscribe() const
{
	cl->unsubscribe(get_nid());
}

void property_fake::request_set(const void *value) const
{
	const property_base *pb = dynamic_cast<const property_base *>(this);

	if(pb == nullptr)
	{
		return;
	}

	cl->request_prop_set_value(pb, value);
}

void client::process_prop_value(const device::package_t &p)
{
	/*const prid_t &prid = p.get_prid();
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
	it->second->notify_change();*/
}


void client::request_prop_set_value(const property_base *p, const void *value)
{
	serializer_base::buffer_t buf = serializer.serialize(p, value);

	if(buf.size() == 0)
	{
		return;
	}

	device::package_t req;

	nid_t nid = 0;
	if(get_prop_nid(p, nid) == false)
	{
		return;
	}

	req.set_nid(nid);
	req.set_cmd(CMD_PROP_SET_VALUE);
	req.append(&(buf[0]), buf.size());

	dev->write(req);
}

bool client::get_prop_nid(const property_base *p, nid_t &nid) const
{
	auto pr = dynamic_cast<const property_fake *>(p);
	if(pr == nullptr)
	{
		return false;
	}

	nid = pr->get_nid();

	return true;

	/*for(props_t::const_iterator it = props.begin() ; it != props.end() ; ++it)
	{
		if(it->second == p)
		{
			prid = it->first;
			return true;
		}
	}

	return false;*/
}

void client::request_add_property(client_node *nd, property_base *prop)
{
//#error
}
