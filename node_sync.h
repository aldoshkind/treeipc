#pragma once

#include <mutex>
#include <thread>

#include "device.h"
#include "package.h"
#include "client_node_value.h"
#include "property_serializer.h"
#include "proxy_node_generator.h"
#include "locking_queue.h"

namespace treeipc
{

class node_sync : public device::listener, public tree_node::listener_t, public property_listener
{
protected:
	device					*dev;
	tree_node				*root_node;
	
	typedef std::map<nid_t, tree_node *>		tracked_t;
	tracked_t									tracked;	
	std::recursive_mutex						tracked_mutex;
	
	locking_queue<const package *> package_queue;
	std::thread				package_processing_thread;
	bool					package_processing_thread_run = false;
	
	void					package_processing_routine		();
	void					process_package					(const package &p);
	
	proxy_node_generator	generator;
	serializer_machine		serializer;
	
	tree_node				*get_node			(nid_t nid);
	property_base			*get_prop			(nid_t prid);
	
	nid_t					get_nid				(tree_node *n);
	bool					get_nid				(tree_node *n, nid_t &nid, bool do_track = true);
	
	nid_t					do_track			(tree_node *n);
	nid_t					do_track			(tree_node *n, nid_t nid);
	void					untrack				(tree_node */*n*/);

	bool					get_nid				(property_base *n, nid_t &nid);
	
	void					cmd_at				(const device::package_t &p);
	void					cmd_ls				(const device::package_t &p);
	void					cmd_get_prop		(const device::package_t &p);
	void					cmd_subscribe		(const device::package_t &p, bool erase = false);
	void					cmd_prop_value		(const device::package_t &p);
	void					cmd_attach			(const device::package_t &p);
	void					cmd_subscribe_add_remove		(const device::package_t &p, bool erase);
	void					cmd_child_added		(const device::package_t &p);
	
	bool					get_prop_nid		(const property_base *p, nid_t &nid) const;
	void					process_prop_value	(const device::package_t &p);

	void					child_added				(tree_node *p, tree_node *n);
	void					child_removed			(tree_node *parent, std::string name, tree_node *child);
	void					process_notification	(const device::package_t *p);
	void					updated					(property_base *prop);
	
	// вынести в класс nid_generator;
	nid_t					current_nid = 0;
	nid_t					generate_nid		();
	
public:
	node_sync();
	~node_sync();
	
	bool					is_server = true;
	
	virtual void			set_device				(device *d);
	void					set_root				(tree_node *root);
	virtual tree_node		*get_root				();

	client_node::ls_list_t	ls						(nid_t nid);
	client_node				*fetch_node				(nid_t nid, std::string name);	
	void					update_prop				(nid_t nid);
	void					subscribe				(nid_t nid);
	void					subscribe_add_remove	(nid_t nid);
	void					unsubscribe				(nid_t nid);
	bool					attach					(nid_t nid, const std::string &name, tree_node *child);
	void					request_prop_set_value	(const property_base *p, const void *value);
};

}
