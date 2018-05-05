#include "stdafx.h"
#include "StringUtils.h"
#include <algorithm>
#include <fstream>
#include <sstream>

std::string StringUtils::Tolower(std::string in) {
	std::transform(in.begin(), in.end(), in.begin(), ::tolower);
	return in;
}

std::vector<std::string> StringUtils::ReadFile(const std::string& filePath) {
	std::vector<std::string> result;
	std::ifstream infile(filePath);
	if (infile.is_open()) {
		std::string str;
		while (infile >> str) {
			result.push_back(str);
		}
	}
	return result;
}

std::vector<std::string> StringUtils::ReadStream(const std::string& content) {
	std::vector<std::string> result;
	std::stringstream ss;
	ss << content;
	std::string str;
	while (ss >> str) {
		result.push_back(str);
	}
	return result;
}

std::string StringUtils::JoinStrings(const std::vector<std::string>& strVec) {
	std::stringstream ss;
	for (unsigned int i = 0; i < strVec.size(); i++) {
		if (i > 0) {
			ss << "\r\n";
		}
		ss << strVec[i];
	}
	return ss.str();
}

std::string StringUtils::ConvertBSTRToMBS(BSTR bstr)
{
	int wslen = ::SysStringLen(bstr);
	return ConvertWCSToMBS((wchar_t*)bstr, wslen);
}

std::string StringUtils::ConvertWCSToMBS(const wchar_t* pstr, long wslen)
{
	int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);
	std::string dblstr(len, '\0');
	len = ::WideCharToMultiByte(CP_ACP, 0,	pstr, wslen, &dblstr[0], len, NULL, NULL );
	return dblstr;
}
