#include <cstdio>
#include <iostream>
#include <nlohmann/json.hpp>
#include "mdb.hpp"
#include "edb.hpp"

using namespace std::literals;
using json = nlohmann::json;

namespace responses
{
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

	void print_response_additional_info(std::string data)
	{
		try
		{
			json j;
			try
			{
				j = json::from_msgpack(data);
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
