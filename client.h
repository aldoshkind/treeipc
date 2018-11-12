#pragma once

#include <mutex>

#include "device.h"
#include "package.h"

#include "client_node.h"
#include "property_serializer.h"

class client;
class client_node;

class proxy_node_factory_base
{
public:
	virtual client_node		*generate						(std::string name) = 0;

	virtual /*destructor*/	~proxy_node_factory_base		() {}
};








class property_fake
{
	nid_t					nid;

	bool					deserialization_in_process;
	client					*cl;

public:
				property_fake			(client *c) : cl(c)
	{
		deserialization_in_process = false;
	}

	virtual 	~property_fake			()
	{
		//
	}

	void					set_deserialization				(bool in_process)
	{
		deserialization_in_process = in_process;
	}

	bool					is_deserialization_in_process	() const
	{
		return deserialization_in_process;
	}

	void					set_client					(client *c)
	{
		cl = c;
	}

	void					set_nid						(nid_t n)
	{
		nid = n;
	}

	nid_t					get_nid						() const
	{
		return nid;
	}

	void					update_value				() const;
	void					subscribe					() const;
	void					unsubscribe					() const;
	void					request_set					(const void *value) const;
};





template <class type>
class property_value_fake : public property_value<type>, public property_fake
{
	typedef property_value<type>		base_t;

public:
	property_value_fake				() : property_fake(nullptr)
	{
		//
	}

	~property_value_fake			()
	{
		//
	}

	void					add_listener					(property_listener *l)
	{
		base_t::add_listener(l);
		if(base_t::get_listeners().size() == 1)
		{
			subscribe();
		}
	}

	void					remove_listener					(property_listener *l)
	{
		base_t::remove_listener(l);
		if(base_t::get_listeners().size() == 0)
		{
			unsubscribe();
		}
	}

	void					set_value						(const type &v)
	{
		base_t::set_value(v);
		/*
		// danger
		if(v == base_t::get_value())
		{
			return;
		}*/

		if(is_deserialization_in_process() == false)
		{
			// report value changed
			request_set(&v);
		}
	}

	type					get_value						() const
	{
		update_value();
		return property_value<type>::get_value();
	}

	using base_t::operator =;
};



template <class T>
class client_node_value : public client_node, public T
{
public:
	client_node_value(nid_t n = 0) : client_node(n)
	{
		//
	}

	void set_nid(nid_t nid)
	{
		T::set_nid(nid);
		client_node::set_nid(nid);
	}
};


template <class T>
class proxy_node_factory : public proxy_node_factory_base
{
	client					*cl;

public:
	/*constructor*/			proxy_node_factory	(client *c) : cl(c)
	{
		//
	}

	/*destructor*/			~proxy_node_factory	()
	{
		//
	}

	client_node				*generate			(std::string name)
	{
		auto node = new client_node_value<property_value_fake<T>>();
		node->property_value_fake<T>::set_client(cl);
		node->client_node::set_client(cl);
		return node;
	}
};




class proxy_node_generator
{
	client					*cl;

	int						init_factories		();

	typedef std::map<std::string, proxy_node_factory_base *>		property_factories_t;
	property_factories_t											property_factories;

public:
	/*constructor*/			proxy_node_generator			(client *c);
	/*destructor*/			~proxy_node_generator			();

	client_node *generate(std::string type, std::string name);
};














class client : public device::listener
{
	friend class client_node;

	device					*dev;
	client_node				*root_node;

	proxy_node_generator	generator;
	serializer_machine		serializer;

	typedef std::map<nid_t, client_node *>		tracked_t;
	tracked_t									tracked;
	std::mutex									tracked_mutex;

	/*typedef std::map<prid_t, property_base *>		props_t;
	props_t											props;*/

	void					process_notification	(const device::package_t &p);
	client_node				*fetch_node				(nid_t nid, std::string name);

	client_node::ls_list_t	ls						(nid_t nid);

	void					process_new_property	(const device::package_t &p);
	void					process_prop_value		(const device::package_t &p);

	//property_base			*generate_property		(std::string type, std::string name);

	bool					get_prop_nid			(const property_base *p, nid_t &prid) const;

public:
	/*constructor*/			client					();
	/*destructor*/			~client					();

	void					set_device				(device *d);
	client_node				*get_root				();
	void					update_prop				(nid_t prid);

	void					subscribe				(nid_t prid);
	void					unsubscribe				(nid_t prid);

	void					request_prop_set_value	(const property_base *p, const void *value);
	void					request_add_property	(client_node *nd, property_base *prop);
};















/*
template <class type>
type property_value_fake<type>::get_value() const
{
	update_value();
	return base_t::get_value();
}
*/
