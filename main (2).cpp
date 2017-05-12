#include <string.h>

#include <iostream>
#include <typeinfo>
#include <map>

#include "tree/node.h"

using namespace std;

class device
{
public:
	/*constructor*/			device				()
	{
		listener = NULL;
	}

	virtual /*destructor*/	~device				()
	{
		//
	}

	typedef std::vector<uint8_t>		package_t;

	class data_listener
	{
	public:
		/*constructor*/			data_listener				()
		{
			//
		}

		virtual /*destructor*/	~data_listener				()
		{
			//
		}

		virtual void					data						(const package_t &p) = 0;
	};

	virtual bool				write				(const package_t &p) = 0;

	data_listener			*listener;
};

class pseudodevice : public device
{
	pseudodevice *other;

public:
	/*constructor*/			pseudodevice				()
	{
		other = NULL;
	}

	/*constructor*/			pseudodevice				(pseudodevice *dev)
	{
		other = dev;
		other->set_other(this);
	}

	/*destructor*/			~pseudodevice				()
	{
		//
	}

	void					set_other					(pseudodevice *dev)
	{
		other = dev;
	}

	bool					write						(const package_t &p)
	{
		if(other && other->listener)
		{
			other->listener->data(p);
			return true;
		}
		return false;
	}
};

class server : public node, public device::data_listener, public node::listener_t
{
	void					data				(const device::package_t &p)
	{
		if(p.size() < 1)
		{
			return;
		}

		switch(p[0])
		{
		case 'l':
			if(dev != NULL)
			{
				device::package_t pack;
				pack.resize(1);
				pack[0] = 'r';
				ls_list_t items = target->ls();				int pos = pack.size();
				pack.resize(pack.size() + sizeof(ls_list_t::size_type));
				*(ls_list_t::size_type *)&pack[pos] = items.size();
				for(auto item : items)
				{
					pos = pack.size();
					pack.resize(pack.size() + sizeof(std::string::size_type) + item.size());
					*(std::string::size_type *)&pack[pos] = item.size();
					pos += sizeof(std::string::size_type);
					memcpy(&pack[pos], item.c_str(), item.size());
				}
				dev->write(pack);
			}
		break;
		default:
		break;
		}
	}

	void					child_added			(node *n)
	{
		printf("child added %s\n", n->get_name().c_str());

		if(dev != NULL)
		{
			device::package_t pack;
			pack.resize(1);
			pack[0] = 'a';
			int pos = 0;

			std::string item;
			item = n->get_name();
			pos = pack.size();
			pack.resize(pack.size() + sizeof(std::string::size_type) + item.size());
			*(std::string::size_type *)&pack[pos] = item.size();
			pos += sizeof(std::string::size_type);
			memcpy(&pack[pos], item.c_str(), item.size());

			dev->write(pack);
		}
	}

	void					child_removed		(node *, std::string name)
	{
		//
	}

	device					*dev;
	//node					*target;

public:
	/*constructor*/			server				()
	{
		dev = NULL;
	}

	/*destructor*/			~server				()
	{
		//
	}

	void					set_target			(node *t)
	{
		//target = t;
		t->add_listener(this);
	}

	void					set_device			(device *d)
	{
		dev = d;
	}
};

class client : public node, public device::data_listener
{
	void					data				(const device::package_t &p)
	{
		if(p.size() < 1)
		{
			return;
		}

		switch(p[0])
		{
		case 'r':
			{
				int pos = 1;
				ls_rep.resize(*(ls_list_t::size_type *)&p[pos]);
				pos += sizeof(ls_list_t::size_type);
				for(int i = 0 ; i < ls_rep.size() ; i += 1)
				{
					std::string::size_type st = *(std::string::size_type *)&p[pos];
					pos += sizeof(std::string::size_type);
					std::string str;
					str.resize(st);
					memcpy((void *)str.c_str(), &p[pos], st);
					pos += st;
					ls_rep[i] = str;
				}
			}
		break;
		case 'a':
			{
				int pos = 1;
				std::string::size_type st = *(std::string::size_type *)&p[pos];
				pos += sizeof(std::string::size_type);
				std::string str;
				str.resize(st);
				memcpy((void *)str.c_str(), &p[pos], st);
				generate(str);
				printf("generate %s\n", str.c_str());
			}
		break;
		default:
		break;
		}
	}

	ls_list_t				ls_rep;

	device					*dev;

	typedef uint64_t		nid_t;

	nid_t					get_rep;

	void					append_string		(device::package_t &pack, const std::string &s)
	{
		int pos = pack.size();
		pack.resize(pack.size() + sizeof(std::string::size_type) + s.size());
		memcpy(&pack[pos], s.c_str(), s.size());
	}

	void					create_node			(std::string path, nid_t nid)
	{
		//
	}

public:
	/*constructor*/			client				()
	{
		dev = NULL;
	}

	/*destructor*/			~client				()
	{
		//
	}

	void					set_device			(device *d)
	{
		dev = d;
	}

	ls_list_t				ls					() const
	{
		if(dev == NULL)
		{
			return ls_list_t();
		}

		device::package_t pack;
		pack.resize(1);
		pack[0] = 'l';

		dev->write(pack);

		// тут ожидание ответа

		return ls_rep;
	}

	node					*get				(std::string path) const
	{
		if(dev == NULL)
		{
			return NULL;
		}
		device::package_t pack;
		append_string(pack, path);

		dev->write(pack);

		// ожидание ответа

		if(get_rep == 0)
		{
			return NULL;
		}
		// generate ?


		return res;
	}
};

int main()
{
	pseudodevice pd1;
	pseudodevice pd2(&pd1);

	node root_cl;
	node root_srv;
	root_srv.generate("test");

	client cl;
	server srv;

	cl.set_device(&pd1);
	srv.set_device(&pd2);

	srv.set_target(root_srv.at("test"));

	pd1.listener = &cl;
	pd2.listener = &srv;

	root_srv.generate("test/a");
	root_srv.generate("test/b");
	root_srv.generate("test/c");
	root_srv.generate("test/d");

	root_cl.attach("cl", &cl, false);

	device::package_t pack;
	char test_string[] = "test";
	pack.resize(sizeof(test_string));
	strcpy((char *)&(pack[0]), test_string);

	for(auto entry : cl.ls())
	{
		printf("%s\n", entry.c_str());
	}

	return 0;
}

