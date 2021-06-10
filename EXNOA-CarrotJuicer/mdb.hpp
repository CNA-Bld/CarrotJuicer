#pragma once
#include <string>

namespace mdb
{
	void init();
	void unload();

	std::string find_text(int category, int index);
}
