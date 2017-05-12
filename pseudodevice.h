#pragma once

#include "device.h"

class pseudodevice : public device
{
	pseudodevice *other;

public:
	/*constructor*/			pseudodevice				()
	{
		other = NULL;
	}

	/*constructor*/			pseudodevice				(pseudodevice *dev)
	{
		other = dev;
		other->set_other(this);
	}

	/*destructor*/			~pseudodevice				()
	{
		//
	}

	void					set_other					(pseudodevice *dev)
	{
		other = dev;
	}

	bool					write						(const package_t &p)
	{
		if(other && other->listener)
		{
			other->listener->data(p);
			return true;
		}
		return false;
	}
};
