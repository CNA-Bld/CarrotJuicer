#include "config.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#define CJCONFIG_READ_PROPERTY(field, j, c) if (j.contains(#field)) c.field = j.at(#field)

using json = nlohmann::json;

namespace config
{
	config_struct config = {
		true, true,
		false,
		true,
		false, "", 100, true,
		true, 0,
		true,
	};

	void load()
	{
		if (!std::filesystem::exists("cjconfig.json"))
		{
			return;
		}

		try
		{
			nlohmann::json j;
			std::ifstream i("cjconfig.json");
			i >> j;

			CJCONFIG_READ_PROPERTY(save_request, j, config);
			CJCONFIG_READ_PROPERTY(save_response, j, config);
			CJCONFIG_READ_PROPERTY(print_request, j, config);
			CJCONFIG_READ_PROPERTY(enable_ansi_colors, j, config);
			CJCONFIG_READ_PROPERTY(enable_notifier, j, config);
			CJCONFIG_READ_PROPERTY(notifier_host, j, config);
			CJCONFIG_READ_PROPERTY(notifier_connection_timeout_msec, j, config);
			CJCONFIG_READ_PROPERTY(notifier_print_error, j, config);
			CJCONFIG_READ_PROPERTY(aoharu_team_sort_with_speed, j, config);
			CJCONFIG_READ_PROPERTY(aoharu_print_team_average_status_max_turn, j, config);
			CJCONFIG_READ_PROPERTY(climax_print_shop_items, j, config);

			std::cout << "Loaded cjconfig.json\n";
		}
		catch (const std::exception& e)
		{
			std::cout << "Exception reading cjconfig.json: " << e.what() << "\n";
		}
	}

	const config_struct& get()
	{
		return config;
	}
}
