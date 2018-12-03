#pragma once

#include "tree/resource.h"
#include "property_fake.h"

namespace treeipc
{

template <class type>
class property_value_fake : public property_value<type>, public property_fake
{
	typedef property_value<type>		base_t;

public:
	property_value_fake				() : property_fake(nullptr)
	{
		//
	}

	~property_value_fake			()
	{
		//
	}

	void					add_listener					(property_listener *l)
	{
		base_t::add_listener(l);
		if(base_t::get_listeners().size() == 1)
		{
			subscribe();
		}
	}

	void					remove_listener					(property_listener *l)
	{
		base_t::remove_listener(l);
		if(base_t::get_listeners().size() == 0)
		{
			unsubscribe();
		}
	}

	void					set_value						(const type &v)
	{
		base_t::set_value(v);

		if(is_deserialization_in_process() == false)
		{
			request_set(&v);
		}
	}

	type					get_value						() const
	{
		update_value();
		return property_value<type>::get_value();
	}

	using base_t::operator =;
};

}
