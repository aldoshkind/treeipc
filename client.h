#pragma once

#include "device.h"
#include "package.h"

#include "client_node.h"

class property_factory_base
{
public:
	virtual property_base	*generate					(std::string name) = 0;

	virtual /*destructor*/	~property_factory_base		() {}
};

template <class type>
class property_factory : public property_factory_base
{
public:
	property_base			*generate			(std::string name)
	{
		return new property_value<type>(name);
	}
};

class property_generator
{
	int						init_property_factories		();

	typedef std::map<std::string, property_factory_base *>	property_factories_t;
	property_factories_t									property_factories;

public:
	/*constructor*/			property_generator			();
	/*destructor*/			~property_generator			();

	property_base			*generate					(std::string type, std::string name);
};

class client : public device::data_listener
{
	friend class client_node;

	device					*dev;
	client_node				*root_node;

	property_generator		generator;

	typedef std::map<nid_t, client_node *>		tracked_t;
	tracked_t									tracked;

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
};
