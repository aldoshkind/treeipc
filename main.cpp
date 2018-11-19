#include <string.h>

#include <iostream>
#include <typeinfo>
#include <map>

#include <unistd.h>

#include "client.h"
#include "server.h"

#include "socket_device.h"

#include "property_serializer.h"

#include "io_service.h"
#include "socket_client.h"

#include "acceptor.h"
#include "conn_server.h"

#include "tree/tree_node.h"
#include "tree/tree_node_inherited.h"

int main()
{
	tree_node root;

	acceptor acc;
	conn_server srv(&root);

	acc.set_listener(&srv);
	acc.accept();

	auto nd = new tree_node_inherited<property_value<double>>();
	root.attach("val", nd);

//	tree_node *d = root.at("d");

	/*property_value<double> *pvd0 = new property_value<double>("p0");
	property_value<double> *pvd1 = new property_value<double>("p1");
	property_value<QString> *qstr_prop = new property_value<QString>("text");

	pvd0->set_value(7);
	pvd1->set_value(15);

	d->add_property(pvd0);
	d->add_property(pvd1);
	d->add_property(qstr_prop);*/

	sleep(10000);

	return 0;
}

