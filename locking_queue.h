#pragma once

#include <list>
#include <mutex>
#include <condition_variable>

template <class type>
class locking_queue
{
	std::mutex					mutex;
	std::condition_variable		condvar;

	std::list<type>				elements;

public:
	/*constructor*/				locking_queue			();
	/*destructor*/				~locking_queue			();

	bool						push					(const type &value);
	bool						push_front				(const type &value);
	bool						pop						(type &value);
};

template <class type>
/*constructor*/ locking_queue<type>::locking_queue()
{
	//
}

template <class type>
/*destructor*/ locking_queue<type>::~locking_queue()
{
	//
}

template <class type>
bool locking_queue<type>::push(const type &value)
{
	mutex.lock();
	elements.push_back(value);
	condvar.notify_one();
	mutex.unlock();

	return true;
}

template <class type>
bool locking_queue<type>::push_front(const type &value)
{
	mutex.lock();
	elements.push_front(value);
	condvar.notify_one();
	mutex.unlock();

	return true;
}

template <class type>
bool locking_queue<type>::pop(type &value)
{
	std::unique_lock<decltype(mutex)> lock(mutex);

	for( ; elements.size() < 1 ; )
	{
		condvar.wait(lock);
	}
	value = elements.front();
	elements.pop_front();

	return true;
}
