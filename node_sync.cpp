#include "node_sync.h"

#include "client_node.h"
#include "property_fake.h"

using namespace treeipc;

/*constructor*/ node_sync::node_sync() : generator(this)
{
	dev = NULL;
	root_node = NULL;
	package_processing_thread_run = true;
	package_processing_thread = std::thread(std::bind(&node_sync::package_processing_routine, this));
}

/*destructor*/ node_sync::~node_sync()
{
	package_processing_thread_run = false;
	// вталкиваем пустой пакет чтобы пробудить поток
	package_queue.push(nullptr);
	package_processing_thread.join();
}

void node_sync::set_device(device *d)
{
	dev = d;
}

tree_node *node_sync::get_root()
{
	return root_node;
}

void node_sync::set_root(tree_node *root)
{
	root_node = root;
}

client_node *node_sync::fetch_node(nid_t nid, std::string name)
{
	device::package_t req, rep;
	req.set_cmd(CMD_AT);
	req.set_nid(nid);
	append_string(req, name);

	std::lock_guard<decltype(tracked_mutex)> lg(tracked_mutex);
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

	return dynamic_cast<client_node *>(tracked[rep.get_nid()]);
}

void node_sync::process_package(const package &p)
{
	switch(p.get_cmd())
	{
	case CMD_AT_SUCCESS:
	{
#warning Что это такое? О_о
		client_node *n = new client_node(p.get_nid());
		n->set_client(this);

		std::lock_guard<decltype(tracked_mutex)> lg(tracked_mutex);
		tracked[p.get_nid()] = n;
	}
	break;
	case CMD_PROP_VALUE_UPDATED:
		process_prop_value(p);
	break;
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
	case CMD_NODE_ATTACH:
		cmd_attach(p);
	break;
	break;
	case CMD_SUBSCRIBE_ADD_REMOVE:
		cmd_subscribe_add_remove(p, false);
	break;
	case CMD_CHILD_ADDED:
		cmd_child_added(p);
	break;
	/*case CMD_CHILD_REMOVED:
		cmd_child_added(p);
	break;*/
	default:
	break;
	}
	return;
}

void node_sync::package_processing_routine()
{
	for( ; package_processing_thread_run ; )
	{
		const package *p;
		package_queue.pop(p);
		if(package_processing_thread_run == false || p == nullptr)
		{
			break;
		}
		process_package(*p);
	}
}

void node_sync::process_notification(const device::package_t *p)
{
	if(p == nullptr || p->size() < 1)
	{
		return;
	}
	
	if(p->get_cmd() == CMD_GENERATE_NID)
	{
		package rep;
		rep.set_nid(generate_nid());
		dev->reply(p, rep);
		return;
	}
	
	package_queue.push(p);
	return;
}

client_node::ls_list_t node_sync::ls(nid_t nid)
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
	for(uint32_t i = 0 ; i < count ; i += 1)
	{
		std::string name;
		pos = read_string(rep, name, pos);
		res.push_back(name);
	}

	return res;
}

void node_sync::update_prop(nid_t nid)
{
	std::unique_lock<decltype(tracked_mutex)> lock(tracked_mutex);
	
	tracked_t::iterator it = tracked.find(nid);
	if(it == tracked.end())
	{
		return;
	}
	
	//lock.unlock();
	
	property_fake *pvf = dynamic_cast<property_fake *>(it->second);
	property_base *prop = dynamic_cast<property_base *>(it->second);
	if(pvf == nullptr || pvf->is_deserialization_in_process() || prop == nullptr)
	{
		return;
	}
	
	device::package_t req, rep;

	req.set_nid(nid);
	req.set_cmd(CMD_PROP_GET_VALUE);

	dev->send(req, rep);

	uint32_t sz = rep.read<uint32_t>(0);
	serializer_base::buffer_t buf;
	buf.resize(sz);
	rep.read(sizeof(sz), &(buf[0]), sz);

	pvf->set_deserialization(true);
	serializer.deserialize(buf, prop);
	pvf->set_deserialization(false);

	return;
}

void node_sync::subscribe(nid_t nid)
{
	device::package_t req;

	req.set_nid(nid);
	req.set_cmd(CMD_SUBSCRIBE);

	dev->write(req);
}

void node_sync::subscribe_add_remove(nid_t nid)
{
	device::package_t req;

	req.set_nid(nid);
	req.set_cmd(CMD_SUBSCRIBE_ADD_REMOVE);

	dev->write(req);
}

void node_sync::unsubscribe(nid_t nid)
{
	device::package_t req;

	req.set_nid(nid);
	req.set_cmd(CMD_UNSUBSCRIBE);

	dev->write(req);
}

void node_sync::request_prop_set_value(const property_base *p, const void *value)
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

bool node_sync::get_prop_nid(const property_base *p, nid_t &nid) const
{
	auto pr = dynamic_cast<const property_fake *>(p);
	if(pr == nullptr)
	{
		return false;
	}

	nid = pr->get_nid();

	return true;
}

bool node_sync::attach(nid_t nid, const std::string &/*name*/, tree_node *child)
{
	device::package_t req, rep;
	req.set_cmd(CMD_NODE_ATTACH);
	req.set_nid(nid);
	
	std::string type;
	auto prop = dynamic_cast<property_base *>(child);
	if(prop != nullptr)
	{
		type = prop->get_type();
	}
	
	append_string(req, type);
	append_string(req, child->get_name());

	dev->send(req, rep);

	if(rep.get_cmd() != CMD_SUCCESS)
	{
		return false;
	}
	
	std::lock_guard<decltype(tracked_mutex)> lg(tracked_mutex);	
	tracked[rep.get_nid()] = child;
	
	return true;
}

void node_sync::process_prop_value(const device::package_t &p)
{
	std::unique_lock<decltype(tracked_mutex)> lg(tracked_mutex);
	
	tracked_t::iterator it = tracked.find(p.get_nid());
	if(it == tracked.end())
	{
		return;
	}

	property_fake *pvf = dynamic_cast<property_fake *>(it->second);
	property_base *prop = dynamic_cast<property_base *>(it->second);
	
	lg.unlock();
	if(pvf == nullptr || pvf->is_deserialization_in_process() || prop == nullptr)
	{
		return;
	}

	uint32_t sz = p.read<uint32_t>(0);
	serializer_base::buffer_t buf;
	buf.resize(sz);
	p.read(sizeof(sz), &(buf[0]), sz);

	pvf->set_deserialization(true);
	serializer.deserialize(buf, prop);
	pvf->set_deserialization(false);
}


void node_sync::cmd_ls(const device::package_t &p)
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

void node_sync::cmd_at(const device::package_t &p)
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

	/*if(n != NULL)
	{
		n->add_listener(this);
	}*/
}

void node_sync::cmd_get_prop(const device::package_t &p)
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

void node_sync::cmd_subscribe(const device::package_t &p, bool erase)
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

void node_sync::cmd_prop_value(const device::package_t &p)
{
	nid_t nid = p.get_nid();

	printf("value for %d\n", (int)nid);
	
	std::lock_guard<decltype(tracked_mutex)> lg(tracked_mutex);

	property_base *pb = dynamic_cast<property_base *>(tracked[nid]);

	if(pb == nullptr)
	{
		printf("%s: node with nid %d not found\n", __PRETTY_FUNCTION__, (int)nid);
		return;
	}

	serializer_base::buffer_t buf;
	buf.resize(p.data_size());
	p.read(0, &(buf[0]), buf.size());

	serializer.deserialize(buf, pb);
}

void node_sync::cmd_attach(const device::package_t &p)
{
	device::package_t resp;
	
	std::lock_guard<decltype(tracked_mutex)> lg(tracked_mutex);

	auto it = tracked.find(p.get_nid());
	if(it == tracked.end())
	{
		resp.set_cmd(CMD_ERROR);
	}
	else
	{
		std::string type;
		std::string name;
		int pos = read_string(p, type);
		read_string(p, name, pos);
		
		tree_node *parent = it->second;		

		resp.set_cmd(CMD_SUCCESS);
		nid_t nid = generate_nid();
		resp.set_nid(nid);

		client_node *generated = generator.generate(type, name);
		if(generated == nullptr)
		{
			resp.set_cmd(CMD_ERROR);
			dev->reply(p, resp);
			return;
		}
		generated->set_nid(nid);
		tracked[nid] = generated;
		parent->attach(name, generated);
	}

	dev->reply(p, resp);
}

tree_node *node_sync::get_node(nid_t nid)
{
	std::lock_guard<decltype(tracked_mutex)> lg(tracked_mutex);
	
	tracked_t::iterator it = tracked.find(nid);
	return (it == tracked.end()) ? NULL : it->second;
}

void node_sync::child_added(tree_node *n)
{
	printf("child added %s\n", n->get_name().c_str());

	if(dev != NULL)
	{
		nid_t parent_nid = 0;
		if(get_nid(const_cast<tree_node *>(n->get_parent()), parent_nid) != true)
		{
			return;
		}
		
		client_node *cn = dynamic_cast<client_node *>(n);
		if(cn != nullptr && cn->get_client() == this)
		{
			return;
		}
		
		nid_t nid = do_track(n);
		
		device::package_t pack;
		pack.set_cmd(CMD_CHILD_ADDED);
		pack.set_nid(parent_nid);
		pack.append(nid);

		std::string type;
		property_base *pb = dynamic_cast<property_base *>(n);
		if(pb != nullptr)
		{
			type = pb->get_type();
		}
		
		append_string(pack, n->get_name());
		append_string(pack, type);

		dev->write(pack);
	}
}

void node_sync::child_removed(tree_node *, std::string /*name*/)
{
	//
}

property_base *node_sync::get_prop(nid_t nid)
{
	std::lock_guard<decltype(tracked_mutex)> lg(tracked_mutex);
	
	tracked_t::iterator it = tracked.find(nid);
	if(it == tracked.end())
	{
		return nullptr;
	}

	property_base *pb = dynamic_cast<property_base *>(it->second);

	return pb;
}

bool node_sync::get_nid(tree_node *n, nid_t &nid, bool track)
{
	std::lock_guard<decltype(tracked_mutex)> lg(tracked_mutex);
	
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

nid_t node_sync::get_nid(tree_node *n)
{
	nid_t nid;
	get_nid(n, nid, true);
	return nid;
}

bool node_sync::get_nid(property_base *p, nid_t &nid)
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

nid_t node_sync::do_track(tree_node *n)
{
	nid_t nid = generate_nid();	
	return do_track(n, nid);
}

nid_t node_sync::do_track(tree_node *n, nid_t nid)
{
	std::lock_guard<decltype(tracked_mutex)> lg(tracked_mutex);
	tracked[nid] = n;
	return nid;
}

void node_sync::untrack(tree_node */*n*/)
{
	//
}

void node_sync::updated(property_base *prop)
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

nid_t node_sync::generate_nid()
{
	if(is_server)
	{
		return current_nid++;
	}
	else
	{
		package req, rep;
		req.set_cmd(CMD_GENERATE_NID);
		
		dev->send(req, rep);
				
#warning Добавить проверку
		return rep.get_nid();
	}
}

void node_sync::cmd_subscribe_add_remove(const device::package_t &p, bool erase)
{
	const nid_t &nid = p.get_nid();

	tracked_t::iterator it = tracked.find(nid);
	if(it == tracked.end())
	{
		return;
	}
	tree_node *nd = it->second;

	if(erase == false)
	{
		//props_subscribed.insert(nid);
		nd->add_listener(this);
	}
	else
	{
		//props_subscribed.erase(nid);
		//nd->remove_listener(this);
	}
}

void node_sync::cmd_child_added(const device::package_t &p)
{
	nid_t parent_nid = p.get_nid();
	
	tree_node *parent = get_node(parent_nid);
	
	if(parent == nullptr)
	{
		return;
	}
	
	std::string name, type;
	int pos = 0;
	nid_t nid = p.read<nid_t>(pos);
	pos += sizeof(nid);
	pos = read_string(p, name, pos);
	read_string(p, type, pos);
	
	client_node *nd = generator.generate(type, name);
	if(nd == nullptr)
	{
		return;
	}
	nd->set_nid(nid);
	do_track(nd, nid);
	
	parent->attach(name, nd);
}
