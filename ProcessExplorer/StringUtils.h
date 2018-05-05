#pragma once
#include <vector>
#include <string>
#include <Windows.h>

class StringUtils {
public:
	static std::string Tolower(std::string in);
	static std::vector<std::string> ReadFile(const std::string& filePath);
	static std::vector<std::string> ReadStream(const std::string& content);
	static std::string JoinStrings(const std::vector<std::string>& strVec);
	static std::string ConvertBSTRToMBS(BSTR bstr);
	static std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen);
};






