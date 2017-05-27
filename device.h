#pragma once

#include <stdint.h>
#include <memory.h>

#include <vector>

typedef uint64_t	unique_id_t;			// unique id
typedef unique_id_t		nid_t;			// node id
typedef unique_id_t		prid_t;			// prop id
typedef uint8_t		cmd_t;
typedef uint32_t	msgid_t;

enum CMD
{
	CMD_AT = 1,
	CMD_AT_SUCCESS = 2,
	CMD_AT_ERROR = 3,
	CMD_LS = 4,
	CMD_LS_SUCCESS = 5,
	CMD_LS_ERROR = 6,
	CMD_NEW_PROP = 7,
	CMD_PROP_UPDATE = 8,
	CMD_PROP_UPDATE_ERROR = 9,
	CMD_PROP_UPDATE_SUCCESS = 10,
	CMD_SUBSCRIBE = 11,
	CMD_UNSUBSCRIBE = 12,
	CMD_PROP_VALUE = 13
};

class package : private std::vector<uint8_t>
{
	typedef std::vector<uint8_t> base_t;

	union
	{
		nid_t						*nid;
		prid_t						*prid;
	};
	cmd_t						*cmd;
	msgid_t						*msgid;
	size_type					header_size;

	void						reset_header		()
	{
		nid = (nid_t *)&(*begin());
		cmd = (cmd_t *)(nid + 1);
		msgid = (msgid_t *)(cmd + 1);
		header_size = (uint8_t *)(msgid + 1) - (uint8_t *)nid;
	}

	void						init				()
	{
		extend_by(sizeof(*nid));
		extend_by(sizeof(*cmd));
		extend_by(sizeof(*msgid));
		reset_header();
	}

public:
	/*constructor*/				package				()
	{
		init();
	}

	/*constructor*/				package				(const package &p) : base_t(p)
	{
		init();
		*cmd = p.get_cmd();
		*nid = p.get_nid();
		*msgid = p.get_msgid();
	}

	/*destructor*/				~package			()
	{
		//
	}

	const package				&operator =			(const package op)
	{
		resize(op.size());
		memcpy(&(*begin()), &(op[0]), size());
		reset_header();
		set_nid(op.get_nid());
		set_cmd(op.get_cmd());
		set_msgid(op.get_msgid());

		return *this;
	}

	package						get_resp			()
	{
		package resp;
		resp.set_cmd(get_cmd());
		resp.set_msgid(get_msgid());
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

	nid_t					get_nid					() const
	{
		return *nid;
	}

	nid_t					get_prid					() const
	{
		return *prid;
	}

	cmd_t					get_cmd					() const
	{
		return *cmd;
	}

	msgid_t					get_msgid				() const
	{
		return *msgid;
	}

	void					set_cmd					(cmd_t c)
	{
		*cmd = c;
	}

	void					set_nid					(nid_t n)
	{
		*nid = n;
	}

	void					set_prid				(nid_t n)
	{
		*prid = n;
	}

	void					set_msgid				(msgid_t m)
	{
		*msgid = m;
	}

	using base_t::size;
};

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

	typedef package								package_t;

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

	virtual bool				write				(const package_t &p) = 0;					// just writes package to device
	virtual bool				send				(package_t req, package_t &resp) = 0;		// blocks and waits for response

	data_listener			*listener;
};

