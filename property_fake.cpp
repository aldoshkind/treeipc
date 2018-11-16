#include "property_fake.h"

#include "client.h"

using namespace treeipc;

void property_fake::update_value() const
{
	cl->update_prop(get_nid());
}

void property_fake::subscribe() const
{
	cl->subscribe(get_nid());
}

void property_fake::unsubscribe() const
{
	cl->unsubscribe(get_nid());
}

void property_fake::request_set(const void *value) const
{
	const property_base *pb = dynamic_cast<const property_base *>(this);

	if(pb == nullptr)
	{
		return;
	}

	cl->request_prop_set_value(pb, value);
}
