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

#include "io_service.h"

using namespace std;



int main()
{
	socket_device sd1;
	sd1.listen_on("", 21313);

	node root_srv;
	root_srv.generate("test");

	server srv;

	srv.set_device(&sd1);

	srv.set_target(root_srv.at("test"));

	sd1.set_listener(&srv);

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

	sleep(10000);

	return 0;
}

