#pragma once

#include <vector>
#include <map>
#include <typeinfo>

#include "tree/node.h"

class serializer_base
{
public:
	typedef std::vector<uint8_t>		buffer_t;

	/*constructor*/				serializer_base				()
	{
		//
	}

	virtual /*destructor*/		~serializer_base				()
	{
		//
	}

	virtual buffer_t			serialize					(property_base *c) = 0;
	virtual buffer_t			serialize					(void *c) = 0;
	virtual bool				deserialize					(const buffer_t &buf, property_base *c) = 0;
	virtual bool				deserialize					(const buffer_t &buf, void *c) = 0;
};

template <class type>
class serializer_plain : public serializer_base
{
public:
	/*constructor*/				serializer_plain			()
	{
		//
	}

	/*destructor*/				~serializer_plain			()
	{
		//
	}

	buffer_t					serialize					(property_base *c)
	{
		buffer_t buf;
		property<type> *pd = dynamic_cast<property<type> *>(c);
		if(pd == NULL)
		{
			return buf;
		}

		type value = pd->get_value();

		buf.resize(sizeof(type));
		memcpy(&buf[0], &value, buf.size());

		return buf;
	}

	buffer_t					serialize					(void *c)
	{
		buffer_t buf;
		buf.resize(sizeof(type));
		memcpy(&buf[0], c, buf.size());

		return buf;
	}

	bool						deserialize					(const buffer_t &buf, property_base *c)
	{
		property<type> *pd = dynamic_cast<property<type> *>(c);
		if(pd == NULL || buf.size() != sizeof(type))
		{
			return false;
		}

		type val;
		memcpy(&val, &buf[0], sizeof(val));
		pd->set_value(val);
		return true;
	}

	bool						deserialize					(const buffer_t &buf, void *c)
	{
		if(buf.size() != sizeof(type))
		{
			return false;
		}
		memcpy(c, &buf[0], sizeof(type));
		return true;
	}
};

class serializer_machine
{
	typedef std::map<std::string, serializer_base *>		serializers_t;
	serializers_t									serializers;

	void						init						()
	{
		add_plain_serializer<double>();
		add_plain_serializer<float>();
		add_plain_serializer<int>();
	}

public:
	/*constructor*/				serializer_machine			()
	{
		init();
	}

	/*destructor*/				~serializer_machine			()
	{
		//
	}

	template <class type>
	void						add_plain_serializer		()
	{
		serializers[typeid(type).name()] = new serializer_plain<type>;
	}

	void						add_serializer				(std::string type, serializer_base *s)
	{
		serializers[type] = s;
	}

	serializer_base::buffer_t		serialize					(property_base *c)
	{
		serializers_t::iterator it = serializers.find(c->get_type());
		if(it == serializers.end())
		{
			return serializer_base::buffer_t();
		}
		return it->second->serialize(c);
	}

	template <class type>
	serializer_base::buffer_t		serialize					(type val)
	{
		serializers_t::iterator it = serializers.find(typeid(type).name());
		if(it == serializers.end())
		{
			return serializer_base::buffer_t();
		}
		return it->second->serialize(&val);
	}

	bool						deserialize					(const serializer_base::buffer_t &buf, property_base *c)
	{
		serializers_t::iterator it = serializers.find(c->get_type());
		if(it == serializers.end())
		{
			return false;
		}
		return it->second->deserialize(buf, c);
	}

	template <class type>
	bool						deserialize					(const serializer_base::buffer_t &buf, type &c)
	{
		serializers_t::iterator it = serializers.find(typeid(type).name());
		if(it == serializers.end())
		{
			return false;
		}
		return it->second->deserialize(buf, &c);
	}
};
