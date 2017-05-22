#include <string.h>

#include <iostream>
#include <typeinfo>
#include <map>

#include "tree/node.h"

#include "pseudodevice.h"
#include "client.h"
#include "server.h"

using namespace std;

int main()
{
	pseudodevice pd1;
	pseudodevice pd2(&pd1);

	node root_cl;
	node root_srv;
	root_srv.generate("test");

	fake_node cl;
	server srv;

	cl.set_device(&pd1);
	srv.set_device(&pd2);

	srv.set_target(root_srv.at("test"));

	pd1.listener = &cl;
	pd2.listener = &srv;

	root_srv.generate("test/a/b/c/d/e/f");
	root_srv.generate("test/b");
	root_srv.generate("test/c");
	root_srv.generate("test/d");

	node *d = root_srv.at("test/a/b/c/d");
	d->add_property(new property_value<double>("prop_test0"));
	d->add_property(new property_value<double>("prop_test1"));

	root_cl.attach("cl", &cl, false);

	node *n = cl.at("a/b/c/d");
	printf("at: %08x %s\n", n, n->get_path().c_str());
	for(auto prop : n->get_properties())
	{
		printf("%s %s\n", prop->get_type().c_str(), prop->get_name().c_str());
	}

	for(auto entry : cl.ls())
	{
		printf("%s\n", entry.c_str());
	}

	return 0;
}

