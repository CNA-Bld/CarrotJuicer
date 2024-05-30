#pragma once
#include <string>

namespace mdb
{
	void init();
	void unload();

	std::string find_text(int category, int index);
	const std::pair<std::string, std::string>& get_chara_names(int chara_id);
	// format: " A B |  C D E F", 芝ダ短マ中長, optionally with ANSI color codes
	std::string get_formatted_chara_proper_labels(int chara_id);
	// Item name and description. Name is padded to 40 half-width chars by APPENDING spaces.
	const std::pair<std::string, std::string>& get_item_names(int item_id);
}
