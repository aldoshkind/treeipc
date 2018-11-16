#include "client.h"

using namespace treeipc;

tree_node *client::get_root()
{
	auto root = node_sync::get_root();
	if(root != NULL)
	{
		return root;
	}
	root = fetch_node(0, "/");
	set_root(root);
	return root;
}
