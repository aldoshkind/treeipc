#pragma once

#include <string>

#include "device.h"

namespace treeipc
{

void append_string(device::package_t &pack, const std::string &s);
int read_string(const device::package_t &pack, std::string &s, int pos = 0);

}
