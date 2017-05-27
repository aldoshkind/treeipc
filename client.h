#pragma once

#include "device.h"
#include "package.h"

#include "client_node.h"
#include "property_serializer.h"

class property_factory_base
{
public:
	virtual property_base	*generate					(std::string name) = 0;

	virtual /*destructor*/	~property_factory_base		() {}
};

class client;

class property_fake
{
	prid_t					prid;

	bool					deserialization_in_process;
	client								*cl;

public:
	/*constructor*/			property_fake			(client *c) : cl(c)
	{
		deserialization_in_process = false;
	}

	virtual /*destructor*/	~property_fake			()
	{
		//
	}

	void					set_prid				(prid_t p)
	{
		prid = p;
	}

	prid_t					get_prid				() const
	{
		return prid;
	}

	void					set_deserialization				(bool in_process)
	{
		deserialization_in_process = in_process;
	}

	bool					is_deserialization_in_process	() const
	{
		return deserialization_in_process;
	}
};

template <class type>
class property_value_fake : public property_value<type>, public property_fake
{
	typedef property_value<type>		base_t;

public:
	/*constructor*/			property_value_fake				(std::string name, client *c) : base_t(name), property_fake(c)
	{
		//
	}

	/*destructor*/			~property_value_fake			()
	{
		//
	}

	void					set_client						(client *c)
	{
		cl = c;
	}

	void					add_listener					(property_listener *l)
	{
		base_t::add_listener(l);
		if(base_t::get_listeners().size() == 1)
		{
			// subscribe
		}
	}

	void					remove_listener					(property_listener *l)
	{
		base_t::remove_listener(l);
		if(base_t::get_listeners().size() == 0)
		{
			// unsubscribe
		}
	}

	void					set_value						(const type &v)
	{
		base_t::set_value(v);
		if(is_deserialization_in_process() == false)
		{
			// report value changed
		}
	}

	type					get_value						() const;

	using base_t::operator =;
};

template <class type>
class fake_property_factory : public property_factory_base
{
	client					*cl;

public:
	/*constructor*/			fake_property_factory	(client *c) : cl(c)
	{
		//
	}

	/*destructor*/			~fake_property_factory	()
	{
		//
	}

	property_base			*generate			(std::string name)
	{
		return new property_value_fake<type>(name, cl);
	}
};

class property_generator
{
	client					*cl;

	int						init_property_factories		();

	typedef std::map<std::string, property_factory_base *>	property_factories_t;
	property_factories_t									property_factories;

public:
	/*constructor*/			property_generator			(client *c);
	/*destructor*/			~property_generator			();

	property_base			*generate					(std::string type, std::string name);
};














class client : public device::data_listener
{
	friend class client_node;

	device					*dev;
	client_node				*root_node;

	property_generator		generator;
	serializer_machine		serializer;

	typedef std::map<nid_t, client_node *>		tracked_t;
	tracked_t									tracked;

	typedef std::map<prid_t, property_base *>		props_t;
	props_t											props;

	void					data					(const device::package_t &p);
	client_node				*fetch_node				(nid_t nid, std::string name);

	client_node::ls_list_t	ls						(nid_t nid);

	void					process_new_property	(const device::package_t &p);

	property_base			*generate_property		(std::string type, std::string name);

public:
	/*constructor*/			client					();
	/*destructor*/			~client					();

	void					set_device				(device *d);
	client_node				*get_root				();
	void					update_prop				(prid_t prid);
};


















template <class type>
type property_value_fake<type>::get_value() const
{
	cl->update_prop(get_prid());
	return base_t::get_value();
}
