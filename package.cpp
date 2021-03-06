#include "package.h"

namespace treeipc
{

void append_string(package_stream_base::package_t &pack, const std::string &s)
{
	pack.append<uint32_t>(s.size());
	pack.append((void *)s.c_str(), s.size());
}

int read_string(const package_stream_base::package_t &pack, std::string &s, int pos)
{
	std::string::size_type sz = pack.read<uint32_t>(pos);
	s.resize(sz);
	pos += sizeof(uint32_t);
	pack.read(pos, (void *)(s.c_str()), s.size());
	pos += s.size();
	return pos;
}

}
