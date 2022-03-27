#include <codecvt>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <Windows.h>
#include <SQLiteCpp/SQLiteCpp.h>

#include "config.hpp"


namespace mdb
{
	std::string utf8_encode(const std::wstring& in)
	{
		if (in.empty()) return std::string();
		const int size = WideCharToMultiByte(CP_UTF8, 0, &in[0], in.size(), NULL, 0, NULL, NULL);
		std::string dst(size, 0);
		WideCharToMultiByte(CP_UTF8, 0, &in[0], in.size(), &dst[0], size, NULL, NULL);
		return dst;
	}


	std::wstring utf8_decode(const std::string& in)
	{
		if (in.empty()) return std::wstring();
		const int size = MultiByteToWideChar(CP_UTF8, 0, &in[0], in.size(), NULL, 0);
		std::wstring dst(size, 0);
		MultiByteToWideChar(CP_UTF8, 0, &in[0], in.size(), &dst[0], size);
		return dst;
	}

	SQLite::Database* master;

	void init()
	{
		try
		{
			if (master != nullptr)
			{
				return;
			}

			WCHAR buffer[MAX_PATH];
			const int len = GetEnvironmentVariable(L"USERPROFILE", buffer, MAX_PATH);

			std::wstring path(buffer, len);
			path += L"\\AppData\\LocalLow\\Cygames\\umamusume\\master\\master.mdb";
			master = new SQLite::Database(utf8_encode(path), SQLite::OPEN_READONLY);

			std::cout << "master.mdb opened.\n";
		}
		catch (const std::exception& e)
		{
			std::cout << "Exception opening master.mdb: " << e.what() << "\n";
		}
	}

	void unload()
	{
		try
		{
			if (master == nullptr)
			{
				return;
			}

			delete master;
			master = nullptr;
		}
		catch (std::exception& e)
		{
			std::cout << "Exception unloading master.mdb: " << e.what() << "\n";
		}
	}

	std::string find_text(const int category, const int index)
	{
		if (master == nullptr)
		{
			return "";
		}

		try
		{
			SQLite::Statement query(*master, "SELECT text FROM text_data WHERE category = ? AND \"index\" = ?");
			query.bind(1, category);
			query.bind(2, index);

			while (query.executeStep())
			{
				return query.getColumn(0).getString();
			}
		}
		catch (std::exception& e)
		{
			std::cout << "Exception querying master.mdb: " << e.what() << "\n";
		}
		return "";
	}


	std::unordered_map<int, std::pair<std::string, std::string>> chara_names;
	const std::pair<std::string, std::string> UNKNOWN_CHARA_NAME = {"Unknown", "Unknown"};

	const std::pair<std::string, std::string>& get_chara_names(const int chara_id)
	{
		if (master == nullptr)
		{
			return UNKNOWN_CHARA_NAME;
		}

		if (chara_names.count(chara_id) == 0)
		{
			auto name = find_text(170, chara_id);
			auto cast_name = find_text(7, chara_id);
			if (name.empty() || cast_name.empty())
			{
				return UNKNOWN_CHARA_NAME;
			}

			chara_names[chara_id] = {name, cast_name};
		}
		return chara_names[chara_id];
	}


	const std::unordered_map<int, std::string> proper_labels = {
		{1, "G"}, {2, "F"}, {3, "E"}, {4, "D"}, {5, "C"}, {6, "B"}, {7, "A"}, {8, "S"},
	};

	std::unordered_map<int, std::string> formatted_chara_proper_labels;

	std::string get_formatted_chara_proper_labels(const int chara_id)
	{
		if (master == nullptr)
		{
			return "";
		}

		if (formatted_chara_proper_labels.count(chara_id) == 0)
		{
			SQLite::Statement query(
				*master,
				"SELECT proper_ground_turf, proper_ground_dirt, proper_distance_short, proper_distance_mile, proper_distance_middle, proper_distance_long FROM single_mode_scout_chara WHERE chara_id=? LIMIT 1;");
			query.bind(1, chara_id);

			while (query.executeStep())
			{
				std::string formatted;
				auto values = query.getColumns<std::vector<int>, 6>();

				for (int i = 0; i < 6; i++)
				{
					const bool dim = config::get().enable_ansi_colors && (values[i] < 7);
					formatted += " ";
					formatted += dim ? "\x1b[90m" : "";
					formatted += proper_labels.at(values[i]);
					formatted += dim ? "\x1b[0m" : "";
					if (i == 1)
					{
						formatted += " | ";
					}
				}

				formatted_chara_proper_labels[chara_id] = formatted;
			}
		}
		return formatted_chara_proper_labels[chara_id];
	}

	std::unordered_map<int, std::pair<std::string, std::string>> item_names;

	const std::pair<std::string, std::string>& get_item_names(const int item_id)
	{
		if (master == nullptr)
		{
			return UNKNOWN_CHARA_NAME;
		}
		if (item_names.count(item_id) == 0)
		{
			auto name = find_text(225, item_id);
			auto desc = find_text(238, item_id);
			if (name.empty() || desc.empty())
			{
				return UNKNOWN_CHARA_NAME;
			}

			// Do stupid things to approximate the length. Assume everything other than ASCII takes 2.
			// If anything other than ASCII, Kana or Kanji appears we are probably dead.
			auto utf16 = utf8_decode(name);
			int len = 0;
			for (const auto c : utf16)
			{
				if (c < 0x80)
					len += 1;
				else
					len += 2;
			}
			name.insert(name.end(), 40 - len, ' ');
			item_names[item_id] = {name, desc};
		}
		return item_names[item_id];
	}
}
