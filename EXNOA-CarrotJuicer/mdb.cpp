#include <codecvt>
#include <iostream>
#include <Windows.h>
#include <SQLiteCpp/SQLiteCpp.h>


namespace mdb
{
	std::string utf8_encode(const std::wstring& in)
	{
		if (in.empty()) return std::string();
		int size = WideCharToMultiByte(CP_UTF8, 0, &in[0], in.size(), NULL, 0, NULL, NULL);
		std::string dst(size, 0);
		WideCharToMultiByte(CP_UTF8, 0, &in[0], in.size(), &dst[0], size, NULL, NULL);
		return dst;
	}

	SQLite::Database* master;

	void init()
	{
		try
		{
			WCHAR buffer[MAX_PATH];
			int len = GetEnvironmentVariable(L"USERPROFILE", buffer, MAX_PATH);

			std::wstring path(buffer, len);
			path += L"\\AppData\\LocalLow\\Cygames\\umamusume\\master\\master.mdb";
			master = new SQLite::Database(utf8_encode(path), SQLite::OPEN_READONLY);

			std::cout << "master.mdb opened." << std::endl;
		}
		catch (std::exception& e)
		{
			std::cout << "Exception opening master.mdb: " << e.what() << std::endl;
		}
	}

	std::string find_text(int category, int index)
	{
		if (master == nullptr)
		{
			return "";
		}

		try
		{
			SQLite::Statement query(*master, "SELECT text FROM text_data WHERE category = ? AND \"index\" = ?");
			query.bind(1, category);
			query.bind(2, index);

			while (query.executeStep())
			{
				std::string s = query.getColumn(0);

				return s;
			}
		}
		catch (std::exception& e)
		{
			std::cout << "Exception querying master.mdb: " << e.what() << std::endl;
		}
		return "";
	}
}
