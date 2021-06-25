#include <cstdio>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include "mdb.hpp"
#include "edb.hpp"

using namespace std::literals;
using json = nlohmann::json;

namespace responses
{
	std::string opponent_list_sig =
		"\x81\xB3\x6F\x70\x70\x6F\x6E\x65\x6E\x74\x5F\x69\x6E\x66\x6F\x5F\x61\x72\x72\x61\x79\x93";
	std::string opponent_list_opponent_info_header = "\x88\xC0\x01";
	std::string opponent_list_opponent_info_header_fixed = "\x87";

	json try_parse_msgpack(const std::string& data)
	{
		try
		{
			return json::from_msgpack(data);
		}
		catch (const json::parse_error& e)
		{
			if (e.id == 113)
			{
				// Try to fix team_stadium/opponent_list
				auto idx = data.find(opponent_list_sig);
				if (idx == std::string::npos)
				{
					throw;
				}

				std::string fixed = data;
				int cnt = 0;
				while (true)
				{
					idx = fixed.find(opponent_list_opponent_info_header, idx);
					if (idx == std::string::npos) break;
					fixed.replace(idx, opponent_list_opponent_info_header.length(),
					              opponent_list_opponent_info_header_fixed);
					idx += opponent_list_opponent_info_header_fixed.length();
					cnt += 1;
				}
				if (cnt != 3)
				{
					throw;
				}

				return json::from_msgpack(fixed);
			}

			throw;
		}
	}

	void print_event_data(nlohmann::basic_json<>& e)
	{
		int story_id = e.at("story_id").get<int>();

		std::cout << "event_id = " << e.at("event_id") << "; story_id = " << story_id << std::endl;

		if (auto story_name = mdb::find_text(181, e.at("story_id")); !story_name.empty())
		{
			std::cout << story_name << std::endl;
		}

		if (auto& choice_array = e.at("event_contents_info").at("choice_array"); !choice_array.empty())
		{
			edb::print_choices(story_id);

			std::cout << "choices: ";
			for (auto choice = choice_array.begin(); choice < choice_array.end(); ++choice)
			{
				std::cout << choice.value().at("select_index")
					<< (choice + 1 == choice_array.end() ? "" : ", ");
			}
			std::cout << std::endl;
		}
	}

	const std::map<int, std::string> distance_type_labels = {
		{1, u8"芝短"}, {2, u8"芝マ"}, {3, u8"芝中"}, {4, u8"芝長"}, {5, u8"ダマ"}
	};
	const std::map<int, std::string> distance_type_proper_fields = {
		{1, "proper_distance_short"},
		{2, "proper_distance_mile"},
		{3, "proper_distance_middle"},
		{4, "proper_distance_long"},
		{5, "proper_distance_mile"},
	};

	const std::map<int, std::string> running_style_labels = {{1, u8"逃"}, {2, u8"先"}, {3, u8"差"}, {4, u8"追"}};
	const std::map<int, std::string> running_style_proper_fields = {
		{1, "proper_running_style_nige"},
		{2, "proper_running_style_senko"},
		{3, "proper_running_style_sashi"},
		{4, "proper_running_style_oikomi"},
	};

	const std::map<int, std::string> proper_labels = {
		{1, "G"}, {2, "F"}, {3, "E"}, {4, "D"}, {5, "C"}, {6, "B"}, {7, "A"}, {8, "S"},
	};

	const std::vector<std::pair<std::string, std::string>> status_data_fields = {
		{u8"スピ", "speed"}, {u8"スタ", "stamina"}, {u8"パワ", "power"}, {u8"根性", "guts"}, {u8"賢さ", "wiz"},
		{u8"評価", "rank_score"},
	};

	void print_team_stadium_opponent_info(nlohmann::basic_json<>& o)
	{
		std::cout << "evaluation_point = " << o.at("evaluation_point") << '\n';

		auto& team_data_array = o.at("team_data_array");

		std::map<int, json> trained_chara_map;
		std::vector<json> trained_chara_array;
		for (auto& trained_chara : o.at("trained_chara_array"))
		{
			trained_chara_map[trained_chara.at("trained_chara_id")] = trained_chara;
		}

		std::cout << "    |";
		for (auto& team_data : team_data_array)
		{
			std::cout << distance_type_labels.at(team_data.at("distance_type"))
				<< running_style_labels.at(team_data.at("running_style"))
				<< "|";
			trained_chara_array.push_back(trained_chara_map.at(team_data.at("trained_chara_id")));
		}
		std::cout << '\n';

		std::cout << "    |";
		for (int i = 0; i < team_data_array.size(); ++i)
		{
			auto& team_data = team_data_array[i];
			auto& chara = trained_chara_array[i];
			int distance_type = team_data.at("distance_type");
			std::cout
				<< " " << proper_labels.at(chara.at((distance_type < 5) ? "proper_ground_turf" : "proper_ground_dirt"))
				<< " " << proper_labels.at(chara.at(distance_type_proper_fields.at(distance_type)))
				<< " " << proper_labels.at(chara.at(running_style_proper_fields.at(team_data.at("running_style"))))
				<< "|";
		}
		std::cout << '\n';

		for (auto& field : status_data_fields)
		{
			std::cout << field.first << "|";
			for (auto& chara : trained_chara_array)
			{
				std::cout << std::setw(6) << static_cast<int>(chara.at(field.second)) << "|";
			}
			std::cout << '\n';
		}
		std::cout << '\n';
	}

	void print_response_additional_info(const std::string& data)
	{
		try
		{
			json j;
			try
			{
				j = try_parse_msgpack(data);
			}
			catch (const json::parse_error& e)
			{
				printf("json: parse_error: %s\n", e.what());
				return;
			}

			try
			{
				auto& data = j.at("data");
				if (data.contains("attest") && data.contains("nonce") && data.contains("terms_updated") &&
					data.contains("is_tutorial") && data.contains("resource_version"))
				{
					// tool/start_session, close master.mdb in case the game wants to update it
					std::cout << "Received tool/start_session, unloading master.mdb." << std::endl;
					mdb::unload();
				}
				else if (data.contains("common_define") && data.contains("res_version"))
				{
					// load/index, open master.mdb
					std::cout << "Received load/index, loading master.mdb." << std::endl;
					mdb::init();
				}
				else if (data.contains("unchecked_event_array"))
				{
					// In single mode.
					auto& unchecked_event_array = data.at("unchecked_event_array");
					for (auto& e : unchecked_event_array)
					{
						print_event_data(e);
					}
				}
				else if (data.contains("event_contents_info"))
				{
					// In gallery/play_event.
					print_event_data(data);
				}
				else if (data.contains("opponent_info_array"))
				{
					// team_stadium/opponent_list
					auto& opponent_info_array = j.at("data").at("opponent_info_array");
					for (auto& o : opponent_info_array)
					{
						print_team_stadium_opponent_info(o);
					}
				}
			}
			catch (const json::out_of_range& e)
			{
				// Not a packet that we are interested in, do nothing.
			}
			catch (const json::type_error& e)
			{
				printf("json: type_error: %s\n", e.what());
			}
		}
		catch (...)
		{
			printf("Uncaught exception!\n");
		}
	}
}
