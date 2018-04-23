#include "stdafx.h"

#include <tlhelp32.h>
#include <tchar.h>
#include <algorithm> 
#include "StringUtils.h"
#include "ProcessReader.h"

ProcessInfo::ProcessInfo(int processId, const std::string& processName, const std::string& processDescription, bool special) :
		processId(processId),
		processName(processName),
		processDescription(processDescription),
		special(special)
{}

std::string& ProcessInfo::GetProcessName() {
	return processName;
}

std::string ProcessInfo::GetProcessDescription() {
	return processDescription;
}

int ProcessInfo::GetProcesId() {
	return processId;
}

bool ProcessInfo::IsSpecial() {
	return special;
}

bool ProcessInfo::operator < (const ProcessInfo& other) {
	return (StringUtils::Tolower(processName).compare(StringUtils::Tolower(other.processName)) < 0);
}

ProcessReader::ProcessReader(const std::vector<std::string>& specialProcSubstrs) :
	specialProcSubstrs(specialProcSubstrs)
{}

std::vector<std::string> ProcessReader::GetSpecialProcSubstrs() {
	std::lock_guard<std::mutex> lock(mutex);
	return specialProcSubstrs;
}

void ProcessReader::SetSpecialProcSubstrs(const std::vector<std::string>& specialProcSubstrs) {
	std::lock_guard<std::mutex> lock(mutex);
	this->specialProcSubstrs = specialProcSubstrs;
}

bool ProcessReader::EnableDebugPrivileges() {
	HANDLE hProcess;
	HANDLE hToken;

	if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken)) {
		if (GetLastError() == ERROR_NO_TOKEN) {
			if (!ImpersonateSelf(SecurityImpersonation)) {
				return false;
			}

			if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken)) {
				return false;
			}

			if (!SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)) {
				return false;
			}
		}
		else {
			return false;
		}
	}
	CloseHandle(hToken);

	return true;
}

std::vector<ProcessInfo> ProcessReader::GetProcessList() {
	std::lock_guard<std::mutex> lock(mutex);
	std::vector<ProcessInfo> result = processList;
	return result;
}

void ProcessReader::Update() {
	std::lock_guard<std::mutex> lock(mutex);
	this->processList.clear();

	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	DWORD dwPriorityClass;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap != INVALID_HANDLE_VALUE) {
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hProcessSnap, &pe32)) {
			do {
				std::string processName = std::string(pe32.szExeFile);
				std::string description = GetProcessDescription(pe32.th32ProcessID);
				bool special = checkSpecialProcSubstrs(processName, description);
				this->processList.push_back(ProcessInfo(pe32.th32ProcessID, processName, description, special));

			} while (Process32Next(hProcessSnap, &pe32));
		}
		CloseHandle(hProcessSnap);
	}
	std::sort(processList.begin(), processList.end());
}


bool ProcessReader::checkSpecialProcSubstrs(const std::string& processName, const std::string& description) {
	for (auto specialProcSubstr : specialProcSubstrs) {
		if (StringUtils::Tolower(processName).find(StringUtils::Tolower(specialProcSubstr)) != std::string::npos ||
			StringUtils::Tolower(description).find(StringUtils::Tolower(specialProcSubstr)) != std::string::npos) {
			return true;
		}
	}
	return false;
}

bool ProcessReader::ProcessReader::SetPrivilege(HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege) {
	TOKEN_PRIVILEGES tp = { 0 };
	LUID luid;
	DWORD cb = sizeof(TOKEN_PRIVILEGES);
	if (!LookupPrivilegeValue(NULL, Privilege, &luid)) {
		return false;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege) {
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else {
		tp.Privileges[0].Attributes = 0;
	}
	AdjustTokenPrivileges(hToken, FALSE, &tp, cb, NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS) {
		return false;
	}
	return true;
}

bool ProcessReader::GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough) {
	LPWORD lpwData;
	for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2) {
		if (*lpwData == wLangId) {
			dwId = *((DWORD*)lpwData);
			return true;
		}
	}

	if (!bPrimaryEnough) {
		return false;
	}

	for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2) {
		if (((*lpwData) & 0x00FF) == (wLangId & 0x00FF)) {
			dwId = *((DWORD*)lpwData);
			return true;
		}
	}
	return false;
}

std::string ProcessReader::GetProcessDescription(DWORD th32ProcessID) {
	std::string result = "";
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, th32ProcessID);
	if (hProcess != NULL) {
		TCHAR processExePath[MAX_PATH];
		DWORD value = MAX_PATH;
		if (QueryFullProcessImageName(hProcess, 0, processExePath, &value) != 0) {
			DWORD dwDummy;
			DWORD lpDataSize = GetFileVersionInfoSize(processExePath, &dwDummy);
			LPBYTE lpData = new BYTE[lpDataSize];

			GetFileVersionInfo(processExePath, 0, lpDataSize, lpData);

			LPVOID	lpInfo;
			UINT	unInfoLen;
			VS_FIXEDFILEINFO fileInfo;
			if (VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"), &lpInfo, &unInfoLen)) {

				DWORD	dwLangCode = 0;
				if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, FALSE)) {
					if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, TRUE)) {
						if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), dwLangCode, TRUE)) {
							if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), dwLangCode, TRUE))
								dwLangCode = *((DWORD*)lpInfo);
						}
					}
				}

				char buff[1024];
				sprintf(buff, _T("\\StringFileInfo\\%04X%04X\\FileDescription"), dwLangCode & 0x0000FFFF, (dwLangCode & 0xFFFF0000) >> 16);

				if (VerQueryValue(lpData, buff, &lpInfo, &unInfoLen)) {
					result = std::string((LPCTSTR)lpInfo);
				}
			}
			delete[] lpData;

		}
		CloseHandle(hProcess);
	}
	return result;
}