#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <thread>
#include <windows.h>
#include <nlohmann/json.hpp>
#include <MinHook.h>

using namespace std::literals;
using json = nlohmann::json;

namespace
{
	void create_debug_console()
	{
		AllocConsole();

		FILE* _;
		// open stdout stream
		freopen_s(&_, "CONOUT$", "w", stdout);
		freopen_s(&_, "CONOUT$", "w", stderr);
		freopen_s(&_, "CONIN$", "r", stdin);

		SetConsoleTitle("Umapyoi");

		// set this to avoid turn japanese texts into question mark
		SetConsoleOutputCP(65001);
		std::locale::global(std::locale(""));
	}

	std::string current_time()
	{
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		return std::to_string(ms.count());
	}

	void write_file(std::string file_name, char* buffer, int len)
	{
		FILE* fp;
		fopen_s(&fp, file_name.c_str(), "wb");
		if (fp != nullptr)
		{
			fwrite(buffer, 1, len, fp);
			fclose(fp);
		}
	}

	void print_event_data(nlohmann::basic_json<> e)
	{
		std::cout << "event_id = " << e.at("event_id") << "; story_id = " << e.at("story_id") << std::endl;

		auto choice_array = e.at("event_contents_info").at("choice_array");
		if (!choice_array.empty())
		{
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
				if (data.contains("unchecked_event_array"))
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

	void* LZ4_decompress_safe_ext_orig = nullptr;

	int LZ4_decompress_safe_ext_hook(
		char* src,
		char* dst,
		int compressedSize,
		int dstCapacity)
	{
		int ret = reinterpret_cast<decltype(LZ4_decompress_safe_ext_hook)*>(LZ4_decompress_safe_ext_orig)(
			src, dst, compressedSize, dstCapacity);

		auto out_path = std::string("CarrotJuicer\\").append(current_time()).append("R.msgpack");
		write_file(out_path, dst, ret);
		printf("wrote response to %s\n", out_path.c_str());

		print_response_additional_info(std::string(dst, ret));

		return ret;
	}

	void* LZ4_compress_default_ext_orig = nullptr;

	int LZ4_compress_default_ext_hook(
		char* src,
		char* dst,
		int srcSize,
		int dstCapacity)
	{
		int ret = reinterpret_cast<decltype(LZ4_compress_default_ext_hook)*>(LZ4_compress_default_ext_orig)(
			src, dst, srcSize, dstCapacity);

		auto out_path = std::string("CarrotJuicer\\").append(current_time()).append("Q.msgpack");
		write_file(out_path, src, srcSize);
		printf("wrote request to %s\n", out_path.c_str());

		return ret;
	}

	void bootstrap_carrot_juicer()
	{
		std::filesystem::create_directory("CarrotJuicer");

		auto libnative_module = GetModuleHandle("libnative.dll");
		printf("libnative.dll at %p\n", libnative_module);
		if (libnative_module == nullptr)
		{
			return;
		}

		auto LZ4_decompress_safe_ext_ptr = GetProcAddress(libnative_module, "LZ4_decompress_safe_ext");
		printf("LZ4_decompress_safe_ext at %p\n", LZ4_decompress_safe_ext_ptr);
		if (LZ4_decompress_safe_ext_ptr == nullptr)
		{
			return;
		}
		MH_CreateHook(LZ4_decompress_safe_ext_ptr, LZ4_decompress_safe_ext_hook, &LZ4_decompress_safe_ext_orig);
		MH_EnableHook(LZ4_decompress_safe_ext_ptr);

		auto LZ4_compress_default_ext_ptr = GetProcAddress(libnative_module, "LZ4_compress_default_ext");
		printf("LZ4_compress_default_ext at %p\n", LZ4_compress_default_ext_ptr);
		if (LZ4_compress_default_ext_ptr == nullptr)
		{
			return;
		}
		MH_CreateHook(LZ4_compress_default_ext_ptr, LZ4_compress_default_ext_hook, &LZ4_compress_default_ext_orig);
		MH_EnableHook(LZ4_compress_default_ext_ptr);
	}

	void* load_library_w_orig = nullptr;

	HMODULE __stdcall load_library_w_hook(const wchar_t* path)
	{
		printf("Saw %ls\n", path);

		// GameAssembly.dll code must be loaded and decrypted while loading criware library
		if (path == L"cri_ware_unity.dll"s)
		{
			bootstrap_carrot_juicer();

			MH_DisableHook(LoadLibraryW);
			MH_RemoveHook(LoadLibraryW);

			return LoadLibraryW(path);
		}

		return reinterpret_cast<decltype(LoadLibraryW)*>(load_library_w_orig)(path);
	}
}

void attach()
{
	create_debug_console();

	if (MH_Initialize() != MH_OK)
	{
		printf("Failed to initialize MinHook.\n");
		return;
	}
	printf("MinHook initialized.\n");

	MH_CreateHook(LoadLibraryW, load_library_w_hook, &load_library_w_orig);
	MH_EnableHook(LoadLibraryW);
}

void detach()
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
}
