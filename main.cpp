#include <iostream>
#include <typeinfo>
#include <map>

using namespace std;

class property_base
{
public:
	/*constructor*/				container					()
	{
		//
	}

	virtual /*destructor*/		~container					()
	{
		//
	}
};

template <class type>
class property : public property_base
{
public:
	/*constructor*/				property					()
	{
		//
	}

	/*destructor*/				~property					()
	{
		//
	}

	virtual type				get							() const = 0;
	void						set							(const type &value)	= 0;

	std::string					type						() const
	{
		return typeid(type).name();
	}
};

class serializer
{
public:
	typedef std::vector<uint8_t>		buffer_t;

	/*constructor*/				serializator				()
	{
		//
	}

	virtual /*destructor*/		~serializator				()
	{
		//
	}

	buffer_t					serialize					(property_base *c) = 0;
	void						deserialize					(const buffer_t &buf, property_base *c) = 0;
};

class serializer_machine
{
	std::map<std::string, serializer *>		serializers;

public:
	/*constructor*/				serializer_machine			()
	{
		//
	}

	/*destructor*/				~serializer_machine			()
	{
		//
	}

	void						add_serializer				(std::string type, serializer *s)
	{
		serializer[type] = s;
	}

	serializer::buffer_t		serialize					(property_base *c) = 0;
	void						deserialize					(const buffer_t &buf, property_base *c) = 0;
};

class serializer_double : public serializer
{
public:
	/*constructor*/				serializer_double			()
	{
		//
	}

	/*destructor*/				~serializer_double			()
	{
		//
	}

	buffer_t					serialize					(property_base *c)
	{
		buffer_t buf;
		property<double> *pd = dynamic_cast<property<double> *>(c);
		if(pd == NULL)
		{
			return buf;
		}



		return buf;
	}

	void						deserialize					(const buffer_t &buf, property_base *c)
	{
		//
	}
};

int main()
{
	//
	return 0;
}

