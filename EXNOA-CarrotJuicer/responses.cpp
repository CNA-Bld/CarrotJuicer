#include <cstdio>
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

	json try_parse_msgpack(const std::string* data)
	{
		try
		{
			return json::from_msgpack(*data);
		}
		catch (const json::parse_error& e)
		{
			if (e.id == 113)
			{
				// Try to fix team_stadium/opponent_list
				auto idx = data->find(opponent_list_sig);
				if (idx == std::string::npos)
				{
					throw;
				}

				std::string fixed = *data;
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

	void print_event_data(nlohmann::basic_json<> e)
	{
		int story_id = e.at("story_id").get<int>();

		std::cout << "event_id = " << e.at("event_id") << "; story_id = " << story_id << std::endl;

		auto story_name = mdb::find_text(181, e.at("story_id"));
		if (!story_name.empty())
		{
			std::cout << story_name << std::endl;
		}

		auto choice_array = e.at("event_contents_info").at("choice_array");
		if (!choice_array.empty())
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

	void print_response_additional_info(const std::string* data)
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
				auto data = j.at("data");
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
					auto unchecked_event_array = data.at("unchecked_event_array");
					for (auto iter = unchecked_event_array.begin(); iter < unchecked_event_array.end(); ++iter)
					{
						print_event_data(iter.value());
					}
				}
				else if (data.contains("event_contents_info"))
				{
					// In gallery/play_event.
					print_event_data(data);
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
