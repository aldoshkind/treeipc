#pragma once

#include <memory>

#include "observable.h"

class reliable_bytestream_base : public one_to_one_observable<void, const void * /*data*/, size_t /*data_size*/>
{
	typedef one_to_one_observable<void, void *, size_t> base;

public:
	reliable_bytestream_base()
	{
		//
	}

	virtual ~reliable_bytestream_base()
	{
		//
	}

	virtual void			start			() = 0;

	virtual size_t			write			(const void *data, size_t size) = 0;

protected:
	void					notify_data		(const void *data, size_t size)
	{
		notify(data, size);
	}
};
