#include "client_node.h"

#include "client.h"

/*constructor*/ client_node::client_node(nid_t n)
{
	//dev = NULL;
	nid = n;
}

/*destructor*/ client_node::~client_node()
{
	//
}

/*void					set_device			(device *d)
{
	dev = d;
}*/

void client_node::set_client(client *c)
{
	cl = c;
}

/*property_base *client_node::add_property(property_base *p)
{
	cl->request_add_property(this, p);
	return NULL;
}*/
