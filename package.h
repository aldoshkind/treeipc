#pragma once

#include <string>

#include "package_stream_base.h"

void append_string(package_stream_base::package_t &pack, const std::string &s);
int read_string(const package_stream_base::package_t &pack, std::string &s, int pos = 0);
