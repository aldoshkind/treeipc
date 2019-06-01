#pragma once

#include <vector>
#include <map>
#include <typeinfo>


#include <QString>

#include "tree/tree_node.h"
#include "tree/property.h"

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

	virtual buffer_t			serialize					(property_base *c) const = 0;
	virtual buffer_t			serialize					(const void *c) const = 0;
	virtual bool				serialize					(property_base *from, void *to) const = 0;
	virtual bool				deserialize					(const buffer_t &buf, property_base *c) const = 0;
	virtual bool				deserialize					(const buffer_t &buf, void *c) const = 0;
	virtual int					get_size					() = 0;				// returns size in bytes or -1 if size varies
};

template <class type>
class serializer_plain : public serializer_base
{
public:
	/*constructor*/				serializer_plain			()
	{
		//
	}

    /*destructor*/				~serializer_plain 			()
	{
		//
	}

	buffer_t					serialize					(property_base *c) const
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

	buffer_t					serialize					(const void *c) const
	{
		buffer_t buf;
		buf.resize(sizeof(type));
		memcpy(&buf[0], c, buf.size());

		return buf;
	}
	
	bool						serialize					(property_base *from, void *to) const
	{
		property<type> *pd = dynamic_cast<property<type> *>(from);
		if(pd == NULL)
		{
			return false;
		}

		type value = pd->get_value();
		memcpy(to, &value, sizeof(type));
		
		return true;
	}

	bool						deserialize					(const buffer_t &buf, property_base *c) const
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

	bool						deserialize					(const buffer_t &buf, void *c) const
	{
		if(buf.size() != sizeof(type))
		{
			return false;
		}
		memcpy(c, &buf[0], sizeof(type));
		return true;
	}
	
	int							get_size					()
	{
		return sizeof(type);
	}
};


class serializer_qstring : public serializer_base
{
public:
    /*constructor*/				serializer_qstring			()
    {
        //
    }

    /*destructor*/				~serializer_qstring			()
    {
        //
    }

    buffer_t					serialize					(property_base *c) const
    {
        buffer_t buf;
        property<QString> *pd = dynamic_cast<property<QString> *>(c);
        if(pd == NULL)
		{
            return buf;
        }
        QString value = pd->get_value();

        buf.resize(value.toStdString().size());
        memcpy(&buf[0], value.toStdString().c_str(), value.toStdString().size());
        return buf;
    }
	
	bool						serialize					(property_base *from, void *to) const
    {
        property<QString> *pd = dynamic_cast<property<QString> *>(from);
        if(pd == NULL)
		{
            return false;
        }
        QString value = pd->get_value();

        memcpy(to, value.toStdString().c_str(), value.toStdString().size());

        return true;
    }

    buffer_t					serialize					(const void *c) const
    {
        buffer_t buf;
		QString &value = *((QString *)c);
		buf.resize(value.toStdString().size());
		memcpy(&buf[0], value.toStdString().c_str(), value.toStdString().size());

        return buf;
    }

	bool						deserialize					(const buffer_t &buf, property_base *c) const
	{
		property<QString> *pd = dynamic_cast<property<QString> *>(c);
		if(pd == nullptr) {
			return false;
		}
		QString value(std::string((char*)&buf[0], buf.size()).c_str());
		pd->sync_value(value);
		return true;
	}

	bool						deserialize					(const buffer_t &buf, void *c) const
	{
		if(c == nullptr)
		{
			return false;
		}
		QString &v = *((QString *)c);
		v = QString(std::string((char*)&buf[0], buf.size()).c_str());
        return false;
    }
	
	int							get_size					()
	{
		return -1;
	}
};








class serializer_qbytearray : public serializer_base
{
public:
    /*constructor*/				serializer_qbytearray			()
    {
        //
    }

    /*destructor*/				~serializer_qbytearray			()
    {
        //
    }

    buffer_t					serialize					(property_base *c) const
    {
        buffer_t buf;
        property<QByteArray> *pd = dynamic_cast<property<QByteArray> *>(c);
        if(pd == NULL)
		{
            return buf;
        }
        QByteArray value = pd->get_value();

        buf.resize(value.toStdString().size());
        memcpy(&buf[0], value.toStdString().c_str(), value.toStdString().size());
        return buf;
    }
	
	bool						serialize					(property_base *from, void *to) const
    {
        property<QByteArray> *pd = dynamic_cast<property<QByteArray> *>(from);
        if(pd == NULL)
		{
            return false;
        }
        QByteArray value = pd->get_value();

        memcpy(to, value.data(), value.size());

        return true;
    }

    buffer_t					serialize					(const void *c) const
    {
        buffer_t buf;
		QByteArray &value = *((QByteArray *)c);
		buf.resize(value.size());
		memcpy(&buf[0], value.data(), value.size());

        return buf;
    }

	bool						deserialize					(const buffer_t &buf, property_base *c) const
	{
		property<QByteArray> *pd = dynamic_cast<property<QByteArray> *>(c);
		if(pd == nullptr)
		{
			return false;
		}
		QByteArray value((char *)&buf[0], buf.size());
		pd->sync_value(value);
		return true;
	}

	bool						deserialize					(const buffer_t &buf, void *c) const
	{
		if(c == nullptr)
		{
			return false;
		}
		QByteArray &v = *((QByteArray *)c);
		v = QByteArray((char *)&buf[0], buf.size());
        return false;
    }
	
	int							get_size					()
	{
		return -1;
	}
};











class serializer_machine
{
	typedef std::map<std::string, serializer_base *>		serializers_t;
	serializers_t											serializers;
	mutable std::recursive_mutex							serializers_mutex;

	void						init						()
	{
		std::lock_guard<decltype(serializers_mutex)> lock(serializers_mutex);
		
		add_plain_serializer<double>();
		add_plain_serializer<float>();
		add_plain_serializer<int>();
		add_plain_serializer<bool>();
        add_serializer(typeid(QString).name(), new serializer_qstring);
		add_serializer(typeid(QByteArray).name(), new serializer_qbytearray);
	}

public:
	/*constructor*/				serializer_machine			(bool do_init_default_serializers = true)
	{
		if(do_init_default_serializers)
		{
			init();
		}
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
	
	template <class type>
	void						add_plain_serializer		(const std::string &type_name)
	{
		serializers[type_name] = new serializer_plain<type>;
	}

	void						add_serializer				(std::string type, serializer_base *s)
	{
		serializers[type] = s;
	}

	serializer_base::buffer_t		serialize					(property_base *c)
	{
		std::lock_guard<decltype(serializers_mutex)> lock(serializers_mutex);
		
		serializers_t::iterator it = serializers.find(c->get_type());
		if(it == serializers.end())
		{
			return serializer_base::buffer_t();
		}
		return it->second->serialize(c);
	}

	serializer_base::buffer_t		serialize					(const property_base *c, const void *v) const
	{
		const std::lock_guard<decltype(serializers_mutex)> lock(serializers_mutex);
		
		serializers_t::const_iterator it = serializers.find(c->get_type());
		if(it == serializers.end())
		{
			return serializer_base::buffer_t();
		}
		return it->second->serialize(v);
	}
	
	bool							serialize_to_buffer			(property_base *from, void *to) const
	{
		const std::lock_guard<decltype(serializers_mutex)> lock(serializers_mutex);
		
		serializers_t::const_iterator it = serializers.find(from->get_type());
		if(it == serializers.end())
		{
			return false;
		}
		return it->second->serialize(from, to);
	}

	template <class type>
	serializer_base::buffer_t		serialize					(type val)
	{
		std::lock_guard<decltype(serializers_mutex)> lock(serializers_mutex);
		
		serializers_t::iterator it = serializers.find(typeid(type).name());
		if(it == serializers.end())
		{
			return serializer_base::buffer_t();
		}
		return it->second->serialize(&val);
	}

	bool						deserialize					(const serializer_base::buffer_t &buf, property_base *c)
	{
#warning
		std::unique_lock<decltype(serializers_mutex)> lock(serializers_mutex);
		
		serializers_t::iterator it = serializers.find(c->get_type());
		if(it == serializers.end())
		{
			return false;
		}
		lock.unlock();
		return it->second->deserialize(buf, c);
	}

	template <class type>
	bool						deserialize					(const serializer_base::buffer_t &buf, type &c)
	{
#warning
		std::unique_lock<decltype(serializers_mutex)> lock(serializers_mutex);
		
		serializers_t::iterator it = serializers.find(typeid(type).name());
		if(it == serializers.end())
		{
			return false;
		}
		lock.unlock();
		return it->second->deserialize(buf, &c);
	}
	
	int							get_size					(const std::string &type)
	{
		const std::lock_guard<decltype(serializers_mutex)> lock(serializers_mutex);
		
		serializers_t::const_iterator it = serializers.find(type);
		if(it == serializers.end())
		{
			return -1;
		}
		return it->second->get_size();
	}
};
