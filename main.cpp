#include <string.h>

#include <iostream>
#include <typeinfo>
#include <map>

#include <unistd.h>

#include "tree/node.h"

#include "pseudodevice.h"
#include "client.h"
#include "server.h"

#include "socket_device.h"

#include "property_serializer.h"

using namespace std;

class prop_change_printer : public property_listener
{
public:
	/*constructor*/			prop_change_printer			()
	{
		//
	}

	/*destructor*/			~prop_change_printer		()
	{
		//
	}

	void					updated						(property_base *prop)
	{
		double val = 0;
		if(prop->get_type() == "d")
		{
			val = dynamic_cast<property<double> *>(prop)->get_value();
		}
		printf("value %s of type %s changed %f\n", prop->get_name().c_str(), prop->get_type().c_str(), val);
	}
};

int main()
{
	char test[] = "ass";


	socket_client sc("127.0.0.1", 21313);
	bool result = sc.write(test, sizeof(test));

	return 0;



	/*pseudodevice pd1;
	pseudodevice pd2(&pd1);*/

	socket_device sd1;
	//sd1.listen_on("", 21313);

	sleep(1);

	socket_device sd2;
	sd2.set_server("", 21313);

	node root_cl;
	node root_srv;
	root_srv.generate("test");

	client cl;
	server srv;

	cl.set_device(&sd1);
	srv.set_device(&sd2);

	srv.set_target(root_srv.at("test"));

	sd1.set_listener(&cl);
	sd2.set_listener(&srv);

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

	prop_change_printer pcp;

	property_base *plast = NULL;
	for(auto prop : n->get_properties())
	{
		printf("%s %s\n", prop->get_type().c_str(), prop->get_name().c_str());
		prop->add_listener(&pcp);

		property<double> *pd = dynamic_cast<property<double> *>(prop);
		if(pd != NULL)
		{
			printf("\t%f\n", pd->get_value());
		}
		plast = prop;
	}

	for(auto entry : cl_root->ls())
	{
		printf("%s\n", entry.c_str());
	}

	/*for(int i = 0 ; i < 5 ; i += 1)
	{
		pvd0->set_value(i / 100.0);
		pvd1->set_value(i * 3.33);
		sleep(1);

		if(i == 2)
		{
			plast->remove_listener(&pcp);
		}
	}*/

	return 0;
}

