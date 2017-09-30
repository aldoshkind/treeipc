#pragma once

#include <map>
#include <set>

#include "tree/node.h"
#include "device.h"

#include "package.h"

#include "property_serializer.h"

class server : public device::listener, public node::new_property_listener, public property_listener
{
	typedef std::map<nid_t, node *>		tracked_t;
	tracked_t							tracked;

	typedef std::map<prid_t, property_base *>		props_t;
	props_t											props;

	typedef std::set<prid_t>						props_subscribed_t;
	props_subscribed_t								props_subscribed;

	serializer_machine		serializer;

	node					*get_node			(nid_t nid);
	property_base			*get_prop			(prid_t prid);

	nid_t					get_nid				(node *n);
	bool					get_nid				(node *n, nid_t &nid, bool do_track = true);

	bool					get_prid			(property_base *n, prid_t &prid);

	void					cmd_at				(const device::package_t &p);
	void					cmd_ls				(const device::package_t &p);
	void					cmd_get_prop		(const device::package_t &p);
	void					cmd_subscribe		(const device::package_t &p, bool erase = false);
	void					cmd_prop_value		(const device::package_t &p);

	void					process_notification(const device::package_t &p);
	void					child_added			(node *n);
	void					child_removed		(node *, std::string name);

	device					*dev;
	node					*target;
	nid_t					current_nid;
	prid_t					current_prid;

	nid_t					generate_nid		();
	prid_t					generate_prid		();
	nid_t					do_track			(node *n);
	void					untrack				(node */*n*/);

	void					new_property		(resource *r, property_base *);
	void					updated				(property_base *prop);

public:
	/*constructor*/			server				();
	/*constructor*/			server				(device *d);
	/*destructor*/			~server				();

	void					set_target			(node *t);
	void					set_device			(device *d);
};
