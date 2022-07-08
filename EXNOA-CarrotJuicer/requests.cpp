#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "config.hpp"

using json = nlohmann::json;


namespace requests
{
	void hex_print(const std::string_view& s)
	{
		for (auto& el : s)
			printf("%02hhx", el);
	}

	const std::vector<int> header_regions = {4, 56, 72, 88, 104, 120};

	void print_request_additional_info(const std::string& data)
	{
		try
		{
			const uint32_t offset = *(uint32_t*)data.c_str();
			if (offset != 166)
			{
				std::cout << "Unknown offset detected: " << offset << "!\n";
			}

			const auto v = std::string_view(data);
			for (int i = 0; i < header_regions.size() - 1; i++)
			{
				hex_print(v.substr(header_regions[i], header_regions[i + 1] - header_regions[i]));
				std::cout << "\n";
			}
			hex_print(v.substr(header_regions[header_regions.size() - 1],
			                   4 + offset - header_regions[header_regions.size() - 1]));
			std::cout << "\n";

			json j;
			try
			{
				j = json::from_msgpack(v.substr(4 + offset));
			}
			catch (const json::parse_error& e)
			{
				std::cout << "json: parse_error: " << e.what() << "\n";
				return;
			}

			try
			{
				std::cout << j.dump(4) << "\n";

				if (j.contains("button_info") && !static_cast<std::string>(j.at("button_info")).empty())
				{
					std::cout << (config::get().enable_ansi_colors ? "\x1b[41m\x1b[93m" : "")
						<< "button_info is non-empty!"
						<< (config::get().enable_ansi_colors ? "\x1b[0m" : "")
						<< "\n";
				}
			}
			catch (const json::out_of_range& e)
			{
				std::cout << "json: out_of_range: " << e.what() << "\n";
			}
			catch (const json::type_error& e)
			{
				std::cout << "json: type_error: " << e.what() << "\n";
			}
		}
		catch (...)
		{
			std::cout << "Uncaught exception!\n";
		}
	}
}
