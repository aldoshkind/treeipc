#pragma once

#include <memory>

#include "observable.h"

class reliable_serial : public one_to_one_observable<void, const void * /*data*/, size_t /*data_size*/>
{
	typedef one_to_one_observable<void, void *, size_t> base;

public:
	reliable_serial()
	{
		//
	}

	virtual ~reliable_serial()
	{
		//
	}

	virtual size_t			write			(const void *data, size_t size) = 0;

protected:
	void					notify_data		(const void *data, size_t size)
	{
		notify(data, size);
	}

};
