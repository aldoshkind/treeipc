#pragma once

class serializer
{
public:
	typedef std::vector<uint8_t>		buffer_t;

	/*constructor*/				serializer				()
	{
		//
	}

	virtual /*destructor*/		~serializer				()
	{
		//
	}

	virtual buffer_t					serialize					(property_base *c) = 0;
	virtual void						deserialize					(const buffer_t &buf, property_base *c) = 0;
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
		serializers[type] = s;
	}

	virtual serializer::buffer_t		serialize					(property_base *c) = 0;
	virtual void						deserialize					(const serializer::buffer_t &buf, property_base *c) = 0;
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
