#include "client_nonroot.h"

using namespace treeipc;

void client_nonroot::set_root(tree_node *root)
{
	nid_t root_nid = do_track(root, 0);
	// запросить подписку на обновления
	subscribe_add_remove(root_nid);
	root->add_listener(this, false);
}

void client_nonroot::child_added(tree_node *p, tree_node *n)
{
	return;
	
	client_node *cn = dynamic_cast<client_node *>(n);
	if(cn != nullptr && cn->get_client() == this)
	{
		printf("child rejected %s\n", n->get_name().c_str());
		return;
	}
	
	if(n->get_parent() == get_root())
	{
		printf("child rejected %s\n", n->get_name().c_str());
		return;
	}
	node_sync::child_added(p, n);
}
