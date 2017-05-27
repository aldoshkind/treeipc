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

	property_value<double> *pvd0 = new property_value<double>("prop_test0");
	property_value<double> *pvd1 = new property_value<double>("prop_test1");

	pvd0->set_value(7);
	pvd1->set_value(15);

	d->add_property(pvd0);
	d->add_property(pvd1);



	node *cl_root = cl.get_root();

	node *n = cl_root->at("a/b/c/d");
	printf("at: %08x %s\n", (int)(int64_t)n, n->get_path().c_str());
	for(auto prop : n->get_properties())
	{
		printf("%s %s\n", prop->get_type().c_str(), prop->get_name().c_str());

		property<double> *pd = dynamic_cast<property<double> *>(prop);
		if(pd != NULL)
		{
			printf("\t%f\n", pd->get_value());
		}
	}

	printf("\n");

	for(auto entry : cl_root->ls())
	{
		printf("%s\n", entry.c_str());
	}

	return 0;
}

