#pragma once

#include <stdint.h>

#include <vector>

typedef uint64_t	nid_t;
typedef uint8_t		cmd_t;
typedef uint32_t	msgid_t;

enum CMD
{
	CMD_AT = 1,
	CMD_AT_SUCCESS = 2,
	CMD_AT_ERROR = 3,
	CMD_LS = 4,
	CMD_LS_SUCCESS = 5,
	CMD_LS_ERROR = 6
};

class package : private std::vector<uint8_t>
{
	typedef std::vector<uint8_t> base_t;

	nid_t						*nid;
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

	package						get_resp			()
	{
		package resp;
		resp.set_cmd(get_cmd());
		resp.set_msgid(get_msgid());
		resp.set_nid(get_nid());
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

	virtual bool				write				(const package_t &p) = 0;
	virtual bool				send				(package_t req, package_t &resp) = 0;		// blocks

	data_listener			*listener;
};
