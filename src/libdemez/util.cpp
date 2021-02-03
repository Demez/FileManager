#include "util.h"


#ifdef _WIN32
	#include <direct.h>
	#include <sysinfoapi.h>
	#include <io.h>
	#include <synchapi.h>
	#include <fileapi.h>

	// get rid of the dumb windows posix depreciation warnings
	#define mkdir _mkdir
	#define chdir _chdir
	#define access _access
#elif __linux__
	#include <stdlib.h>
	#include <unistd.h>

	// windows-specific mkdir() is used
	#define mkdir(f) mkdir(f, 666)
#endif


size_t str_count(std::string string, std::string item)
{
	size_t count = 0;

	while (string.find(item) != std::string::npos)
	{
		string = string.substr(string.find(item) + 1);
		count++;
	}

	return count;
}


void str_replace(std::string& str, const std::string& from, const std::string& to)
{
    if (str == "")
        return;

    while (str.find(from) != std::string::npos)
    {
        size_t start_pos = str.find(from);
        if(start_pos == std::string::npos)
            return;

        str.replace(start_pos, from.length(), to);
    }
}


void str_replace(std::wstring& str, const std::wstring& from, const std::wstring& to)
{
    if (str == L"")
        return;

    while (str.find(from) != std::wstring::npos)
    {
        size_t start_pos = str.find(from);
        if(start_pos == std::wstring::npos)
            return;

        str.replace(start_pos, from.length(), to);
    }
}


inline std::string RemoveFileExt(std::string &path)
{
	fs::path fspath = path;

	if (fspath.has_extension())
	{
		return path.substr(0, path.length() - fspath.extension().string().length()); 
	}

	return path;
}


void GetLongestString(std::vector<std::string> &strVec, std::string &longest)
{
	longest = strVec[0];
	for (std::string str: strVec)
	{
		if (str.length() > longest.length())
			longest = str;
	}
}


std::string GetLongestString(std::vector<std::string> &strVec)
{
	std::string longest;
	GetLongestString(strVec, longest);
	return longest;
}


int ChangeDir(std::string &path)
{
	return chdir(path.c_str());
}

int CreateDir(std::string &path)
{
	return mkdir(path.c_str());
}

std::string GetCurrentDir()
{
#ifdef _WIN32
    std::string cwd = _getcwd( NULL, 0 );
    str_replace(cwd, "\\", "/");
	return cwd;
#else
	return getcwd( NULL, 0 );
#endif
}


bool ItemExists(const std::string &path)
{
	return (access(path.c_str(), 0) != -1);
}


const char* PathToChar(const fs::path& path)
{
    static std::string result;
    result = path.string();
    return result.c_str();
}


const wchar_t* PathToWChar(const fs::path& path)
{
    static std::wstring result;
    result = path.wstring();
	return result.c_str();
}


void OS_Sleep(int ms)
{
#ifdef _WIN32
	Sleep(ms);
#elif __linux__
	usleep(ms);
#endif
}


bool OS_IsJunction(const fs::path dir)
{
#ifdef _WIN32

	// HANDLE
	// bool ret = GetFileInformationByHandle(hFile, lpFileInformation)

	return false;

#else
	return false;
#endif
}


bool IsDirSafe(const fs::path &dir)
{
    try
    {
        return fs::is_directory(dir);
    }
    catch (fs::filesystem_error)
    {
        return false;
    }
}



