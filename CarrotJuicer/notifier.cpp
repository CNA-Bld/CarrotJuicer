#include <string>
#include <httplib.h>

#include "config.hpp"

namespace notifier
{
	httplib::Client* client = nullptr;

	void init()
	{
		auto& c = config::get();

		if (!c.enable_notifier) return;

		client = new httplib::Client(c.notifier_host.data());
		client->set_connection_timeout(0, c.notifier_connection_timeout_msec * 1000);
	}

	void notify_response(const std::string& data)
	{
		if (client == nullptr) return;

		if (auto res = client->Post("/notify/response", data, "application/x-msgpack"))
		{
			if (res->status != 200)
			{
				std::cout << "Unexpected response from listener: " << res->status << "\n";
			}
		}
		else
		{
			if (config::get().notifier_print_error)
			{
				std::cout << "Failed to notify listener: " << res.error() << "\n";
			}
		}
	}
}
