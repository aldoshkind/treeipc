#pragma once

#include <mutex>

template <class RV, class ... ARGS>
class one_to_one_observable
{
public:
	class listener
	{
		std::mutex mut;
		one_to_one_observable *obs;

	public:
		listener()
		{
			obs = nullptr;
		}

		virtual ~listener()
		{
			set_observable(nullptr);
		}

		virtual RV process_notification(ARGS ...) = 0;/*
		{
			return RV();
		}*/

		void set_observable(one_to_one_observable *o)
		{
			std::lock_guard<std::mutex> lg(mut);
			if(obs != nullptr)
			{
				obs->remove_listener();
			}
			obs = o;
		}

		void remove_observable()
		{
			std::lock_guard<std::mutex> lg(mut);
			obs = nullptr;
		}
	};

public:
	RV notify(ARGS ... a)
	{
		std::lock_guard<std::mutex> lg(mut);
		if(l != nullptr)
		{
			return l->process_notification(a ...);
		}
	}
private:

	std::mutex		mut;
	listener		*l;

public:

	one_to_one_observable()
	{
		l = nullptr;
	}

	~one_to_one_observable()
	{
		std::lock_guard<std::mutex> lg(mut);
		if(l != nullptr)
		{
			l->remove_observable();
		}
	}

	void set_listener(listener *l)
	{
		std::lock_guard<std::mutex> lg(mut);
		if(this->l != nullptr)
		{
			this->l->remove_observable();
		}
		this->l = l;
		if(l != nullptr)
		{
			l->set_observable(this);
		}
	}

	void remove_listener()
	{
		std::lock_guard<std::mutex> lg(mut);
		l = nullptr;
	}
protected:
	listener *get_listener()
	{
		return l;
	}
};
