#pragma once

#include "device.h"

class pseudodevice : public device
{
	pseudodevice *other;

	package in;

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

	void					accept						(const package &p)
	{
		in = p;
		accept_data(p);
	}

	bool					write						(const package_t &p)
	{
		if(other)
		{
			other->accept(p);
			return true;
		}
		return false;
	}

	bool					send						(package_t req, package_t &resp)
	{
		req.set_msgid(0);
		bool res = write(req);
		if(res == false)
		{
			return false;
		}
		resp = in;
		return true;
	}
};
