#pragma once
#include <string>

namespace config
{
	struct config_struct
	{
		bool save_request;
		bool save_response;

		bool print_request;

		bool enable_ansi_colors;

		bool enable_notifier;
		std::string notifier_host;
		int notifier_connection_timeout_msec;
		bool notifier_print_error;

		bool aoharu_team_sort_with_speed;
		int aoharu_print_team_average_status_max_turn;

		bool climax_print_shop_items;
	};

	void load();

	config_struct const& get();
}
