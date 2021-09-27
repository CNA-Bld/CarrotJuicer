#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


namespace edb
{
	std::map<int, json> events_map;

	void init()
	{
		if (!std::filesystem::exists("cjedb.json"))
		{
			std::cout << "Skipping cjedb.json.\n";
			return;
		}

		try
		{
			json j;
			std::ifstream i("cjedb.json");
			i >> j;

			const auto& events = j.at("events");
			for (auto it = events.begin(); it < events.end(); ++it)
			{
				events_map[it.value().at("storyId")] = it.value();
			}

			std::cout << "cjedb.json opened, read " << events_map.size() << " events.\n";
		}
		catch (std::exception& e)
		{
			std::cout << "Exception reading cjedb.json: " << e.what() << "\n";
		}
	}

	void print_choices(const int story_id)
	{
		try
		{
			if (const auto search = events_map.find(story_id); search != events_map.end())
			{
				const auto& choice_array = search->second.at("choices");
				for (const auto& choice : choice_array)
				{
					std::cout << "\n" << choice.at("title").get<std::string>() << "\n"
						<< choice.at("text").get<std::string>() << "\n";
				}
				std::cout << "\n";
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "Exception getting choices: " << e.what() << "\n";
		}
	}
}
