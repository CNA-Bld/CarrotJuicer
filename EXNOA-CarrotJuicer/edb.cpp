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
			std::cout << "Skipping cjedb.json." << std::endl;
			return;
		}

		try
		{
			json j;
			std::ifstream i("cjedb.json");
			i >> j;

			auto events = j.at("events");
			for (auto it = events.begin(); it < events.end(); ++it)
			{
				events_map[it.value().at("storyId")] = it.value();
			}

			std::cout << "cjedb.json opened, read " << events_map.size() << " events." << std::endl;
		}
		catch (std::exception& e)
		{
			std::cout << "Exception reading cjedb.json: " << e.what() << std::endl;
		}
	}

	void print_choices(int story_id)
	{
		try
		{
			auto search = events_map.find(story_id);
			if (search != events_map.end())
			{
				auto choice_array = search->second.at("choices");
				for (auto choice = choice_array.begin(); choice < choice_array.end(); ++choice)
				{
					std::cout << std::endl << choice.value().at("title").get<std::string>() << std::endl
						<< choice.value().at("text").get<std::string>() << std::endl;
				}
				std::cout << std::endl;
			}
		}
		catch (std::exception& e)
		{
			std::cout << "Exception getting choices: " << e.what() << std::endl;
		}
	}
}
