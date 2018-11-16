#pragma once

#include "package.h"

namespace treeipc
{

class node_sync;

class property_fake
{
	nid_t					nid;

	bool					deserialization_in_process;
	node_sync					*cl;

public:
				property_fake			(node_sync *c) : cl(c)
	{
		deserialization_in_process = false;
	}

	virtual 	~property_fake			()
	{
		//
	}

	void					set_deserialization				(bool in_process)
	{
		deserialization_in_process = in_process;
	}

	bool					is_deserialization_in_process	() const
	{
		return deserialization_in_process;
	}

	void					set_client					(node_sync *c)
	{
		cl = c;
	}

	void					set_nid						(nid_t n)
	{
		nid = n;
	}

	nid_t					get_nid						() const
	{
		return nid;
	}

	void					update_value				() const;
	void					subscribe					() const;
	void					unsubscribe					() const;
	void					request_set					(const void *value) const;
};

}
