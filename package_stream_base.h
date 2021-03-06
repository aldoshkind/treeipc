#pragma once

#include <stdint.h>
#include <memory.h>

#include <vector>

#include "observable.h"

namespace treeipc
{

typedef uint64_t	unique_id_t;			// unique id
typedef unique_id_t		nid_t;			// node id
typedef uint8_t		cmd_t;

enum CMD
{
	CMD_AT = 1,
	CMD_AT_SUCCESS = 2,
	CMD_AT_ERROR = 3,
	CMD_LS = 4,
	CMD_LS_SUCCESS = 5,
	CMD_LS_ERROR = 6,
	CMD_PROP_GET_VALUE = 8,
	CMD_PROP_GET_VALUE_ERROR = 9,
	CMD_PROP_GET_VALUE_SUCCESS = 10,
	CMD_SUBSCRIBE = 11,
	CMD_UNSUBSCRIBE = 12,
	CMD_PROP_VALUE_UPDATED = 13,
	CMD_PROP_SET_VALUE = 14,
	CMD_SUCCESS = CMD_AT_SUCCESS,
	CMD_ERROR = CMD_AT_ERROR,
	CMD_NODE_ATTACH = 15,
	CMD_NODE_DETACH = 16,
	CMD_GENERATE_NID = 17,
	CMD_CHILD_ADDED = 18,
	CMD_SUBSCRIBE_ADD_REMOVE = 19,
};

class package : private std::vector<uint8_t>
{
	typedef std::vector<uint8_t> base_t;

	nid_t						*nid;
	cmd_t						*cmd;
	size_type					header_size;

	void						init				()
	{
		extend_by(sizeof(*nid));
		extend_by(sizeof(*cmd));
		reset_header();
	}

public:
	/*constructor*/				package				() : base_t()
	{
		init();
	}

	/*constructor*/				package				(const package &p) : base_t(p)
	{
		init();
		*cmd = p.get_cmd();
		*nid = p.get_nid();
	}

	/*destructor*/				~package			()
	{
		//
	}

	void						reset_header		()
	{
		nid = (nid_t *)&(*begin());
		cmd = (cmd_t *)(nid + 1);
		header_size = (uint8_t *)(cmd + 1) - (uint8_t *)nid;
	}

	const package				&operator =			(const package op)
	{
		resize(op.size());
		memcpy(&(*begin()), &(op[0]), size());
		reset_header();
		set_nid(op.get_nid());
		set_cmd(op.get_cmd());

		return *this;
	}

	package						get_resp			()
	{
		package resp;
		resp.set_cmd(get_cmd());
		resp.set_nid(get_nid());

		return resp;
	}

	void						extend_by			(size_type amount)
	{
		resize(size() + amount);
		reset_header();
	}

	template <class type>
	void append(const type &val)
	{
		append(&val, sizeof(val));
	}

	void append(const void *data, int sz)
	{
		size_type pos = size();
		extend_by(sz);
		memcpy(&(*(begin() + pos)), data, sz);
	}

	template <class type>
	type read(size_type pos) const
	{
		type res;
		bool success = read(pos, &res, sizeof(res));
		return success ? res : type();
	}

	bool read(size_type pos, void *dest, size_type sz) const
	{
		pos += header_size;
		if(pos + sz > size())
		{
			return false;
		}

		memcpy(dest, &(*(begin() + pos)), sz);

		return true;
	}

	size_t					data_size				() const
	{
		return base_t::size() - header_size;
	}

	nid_t					get_nid					() const
	{
		return *nid;
	}

	cmd_t					get_cmd					() const
	{
		return *cmd;
	}

	void					set_cmd					(cmd_t c)
	{
		*cmd = c;
	}

	void					set_nid					(nid_t n)
	{
		*nid = n;
	}

	using base_t::size;
	using base_t::push_back;
	using base_t::operator [];
	using base_t::clear;
	using base_t::reserve;
	using base_t::resize;
	using base_t::insert;
	using base_t::erase;
	using base_t::begin;
	using base_t::end;
};

class package_stream_base : public one_to_one_observable<void, const package *>
{
	typedef one_to_one_observable<void, const package *> base_t;
public:
	class listener : public base_t::listener
	{
	public:
		virtual void stream_opened() = 0;
		virtual void stream_closed() = 0;
	};
	
	/*constructor*/			package_stream_base				()
	{
		//
	}

	virtual /*destructor*/	~package_stream_base				()
	{
		//
	}

	typedef	package									package_t;

	virtual bool				write				(const package_t &p) = 0;							// just writes package to device
	virtual bool				send				(package_t req, package_t &resp) = 0;				// blocks and waits for response
	virtual bool				reply				(const package_t *req, const package &rep) = 0;		// replies on req with reply
	bool						reply				(const package_t &req, const package &rep)
	{
		return reply(&req, rep);
	}

	virtual void				start				() = 0;												// starts the process of packages acquision
	
protected:
	void notify_stream_opened()
	{
		auto l = dynamic_cast<listener *>(get_listener());
		if(l != nullptr)
		{
			l->stream_opened();
		}
	}
	
	void notify_stream_closed()
	{
		auto l = dynamic_cast<listener *>(get_listener());
		if(l != nullptr)
		{
			l->stream_closed();
		}
	}
};

}
