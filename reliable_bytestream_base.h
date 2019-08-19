#pragma once

#include <memory>

#include "observable.h"

class reliable_bytestream_base : public one_to_one_observable<void, const void * /*data*/, size_t /*data_size*/>
{
	typedef one_to_one_observable<void, const void *, size_t> base;

public:
	
	class listener : public base::listener
	{
	public:
		virtual void bytestream_opened(){}
		virtual void bytestream_closed(){}
	};
	
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
	
	void notify_bytestream_opened()
	{
		base::listener *bl = get_listener();
		listener *l = dynamic_cast<listener *>(bl);
		if(l != nullptr)
		{
			l->bytestream_opened();
		}
	}
	
	void notify_bytestream_closed()
	{
		base::listener *bl = get_listener();
		listener *l = dynamic_cast<listener *>(bl);
		if(l != nullptr)
		{
			l->bytestream_closed();
		}		
	}
};
