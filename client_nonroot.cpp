#include "client_nonroot.h"

using namespace treeipc;

void client_nonroot::set_root(tree_node *root)
{
	node_sync::set_root(root);
}

void client_nonroot::child_added(tree_node *p, const std::string &name, tree_node *n)
{
	client_node *cn = dynamic_cast<client_node *>(n);
	if(cn != nullptr && cn->get_client() == this)
	{
		printf("child rejected %s\n", name.c_str());
		return;
	}
	
	if(n->get_owner() == get_root())
	{
		printf("child rejected %s\n", name.c_str());
		return;
	}

	node_sync::child_added(p, n);
}

void client_nonroot::stream_opened()
{
	// запросить подписку на обновления
	printf("%s\n", __func__);
	do_track(get_root(), 0);
	subscribe_add_remove(0);				// 0 это nid для root
	tree_node *root = get_root();
	if(root != nullptr)
	{
		root->add_listener(this, false);
	}
}
