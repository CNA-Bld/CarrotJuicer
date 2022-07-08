#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


namespace edb
{
	std::map<int, std::string> formatted_events_choices;

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
				const auto& v = it.value();
				std::stringstream formatted;

				for (const auto& choice : v.at("choices"))
				{
					formatted << "\n" << choice.at("title").get<std::string>() << "\n"
						<< choice.at("text").get<std::string>() << "\n";
				}
				formatted << "\n";

				formatted_events_choices[v.at("storyId")] = formatted.str();
			}

			std::cout << "cjedb.json opened, read " << formatted_events_choices.size() << " events.\n";
		}
		catch (std::exception& e)
		{
			std::cout << "Exception reading cjedb.json: " << e.what() << "\n";
		}
	}

	void print_choices(const int story_id)
	{
		if (const auto search = formatted_events_choices.find(story_id); search != formatted_events_choices.end())
		{
			std::cout << search->second;
		}
	}
}
