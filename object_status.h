#pragma once

#include <mutex>
#include <memory>

class object_status
{
public:
	std::mutex mut;
	bool ok;

	typedef std::shared_ptr<object_status> sp;

	object_status()
	{
		set_ok(true);
	}

	bool is_ok() const
	{
		return ok;
	}

	void set_ok(bool ok)
	{
		this->ok = ok;
	}
};
