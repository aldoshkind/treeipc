#include <string.h>

#include <iostream>
#include <typeinfo>
#include <map>

#include "tree/node.h"

#include "pseudodevice.h"
#include "client.h"
#include "server.h"

#include "property_serializer.h"

using namespace std;

int main()
{
	pseudodevice pd1;
	pseudodevice pd2(&pd1);

	node root_cl;
	node root_srv;
	root_srv.generate("test");

	client cl;
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



	node *cl_root = cl.get_root();

	node *n = cl_root->at("a/b/c/d");
	printf("at: %08x %s\n", n, n->get_path().c_str());
	for(auto prop : n->get_properties())
	{
		printf("%s %s\n", prop->get_type().c_str(), prop->get_name().c_str());
	}

	printf("\n");

	for(auto entry : cl_root->ls())
	{
		printf("%s\n", entry.c_str());
	}

	serializer_machine m;
	serializer::buffer_t buf = m.serialize(7.0f);

	float f;
	m.deserialize(buf, f);

	return 0;
}

