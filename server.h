#pragma once

#include <map>

#include "tree/node.h"
#include "device.h"

#include "package.h"

class server : public device::data_listener, public node::new_property_listener
{
	typedef std::map<nid_t, node *>		tracked_t;
	tracked_t							tracked;

	node					*get_node			(nid_t nid);

	nid_t					get_nid				(node *n);
	bool					get_nid				(node *n, nid_t &nid, bool do_track = true);

	void					cmd_at				(const device::package_t &p);
	void					cmd_ls				(const device::package_t &p);

	void					data				(const device::package_t &p);
	void					child_added			(node *n);
	void					child_removed		(node *, std::string name);

	device					*dev;
	node					*target;
	nid_t					current_nid;

	nid_t					generate_nid		();
	nid_t					do_track			(node *n);
	void					untrack				(node */*n*/);

	void					new_property		(resource *r, property_base *);

public:
	/*constructor*/			server				();
	/*destructor*/			~server				();

	void					set_target			(node *t);
	void					set_device			(device *d);
};
