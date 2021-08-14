#pragma once
#include <string>

namespace config
{
	struct config_struct
	{
		bool save_request;
		bool save_response;

		bool enable_notifier;
		std::string notifier_host;
		int notifier_connection_timeout_msec;
	};

	void load();

	config_struct& get();
}
