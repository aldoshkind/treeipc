#include "proxy_node_generator.h"

#include "proxy_node_factory.h"

#include <QString>

using namespace treeipc;

class untyped_node_factory : public proxy_node_factory_base
{
	node_sync *cl;
public:
	untyped_node_factory(node_sync *c) : cl(c)
	{
		//
	}

	client_node		*generate			(std::string/* name*/)
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


/*constructor*/ proxy_node_generator::proxy_node_generator(node_sync *c) : cl(c)
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
