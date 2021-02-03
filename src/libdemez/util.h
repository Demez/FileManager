#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;


template <class T>
size_t vec_index(std::vector<T> &vec, T item)
{
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (vec[i] == item)
			return i;
	}

	return SIZE_MAX;
}


template <class T>
size_t vec_index(const std::vector<T> &vec, T item)
{
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (vec[i] == item)
			return i;
	}

	return SIZE_MAX;
}


template <class T>
void vec_remove(std::vector<T> &vec, T item)
{
	vec.erase(vec.begin() + vec_index(vec, item));
}


template <class T>
bool vec_contains(std::vector<T> &vec, T item)
{
	for (T addedItem: vec)
	{
		if (addedItem == item)
			return true;
	}

	return false;
}


template <class T>
bool vec_contains(const std::vector<T> &vec, T item)
{
	for (T addedItem: vec)
	{
		if (addedItem == item)
			return true;
	}

	return false;
}


size_t str_count(std::string string, std::string item);

void str_replace(std::string& str, const std::string& from, const std::string& to);
void str_replace(std::wstring& str, const std::wstring& from, const std::wstring& to);


/*template<class T>
void str_replace(std::basic_string<T>& str, const std::basic_string<T>& from, const std::basic_string<T>& to)
{
    if (str == "")
        return;

    while (str.find(from) != std::basic_string<T>::npos)
    {
        size_t start_pos = str.find(from);
        if(start_pos == std::basic_string<T>::npos)
            return;

        str.replace(start_pos, from.length(), to);
    }
}*/


inline void str_upper(std::string &string)
{
	std::transform(string.begin(), string.end(), string.begin(), ::toupper);
}

inline void str_lower(std::string &string)
{
	std::transform(string.begin(), string.end(), string.begin(), ::tolower);
}


std::string RemoveFileExt(std::string &path);

const char* PathToChar(const fs::path& path);
const wchar_t* PathToWChar(const fs::path& path);

void OS_Sleep(int ms);
bool OS_IsJunction(const fs::path &dir);

bool IsDirSafe(const fs::path &dir);


