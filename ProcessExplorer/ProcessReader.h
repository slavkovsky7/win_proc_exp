#pragma once
#include <mutex>
#include <vector>
#include <string>

class ProcessInfo {
private:
	int processId;
	std::string processName;
	std::string processDescription;
	bool special;
public:
	ProcessInfo(int processId, const std::string& processName, const std::string& processDescription, bool special);
	std::string& GetProcessName();
	std::string GetProcessDescription();
	int GetProcesId();
	bool IsSpecial();
	bool operator < (const ProcessInfo& other);
};

class ProcessReader {
private:
	std::vector<ProcessInfo> processList;
	std::vector<std::string> specialProcSubstrs;
	std::mutex mutex;
public:
	ProcessReader(const std::vector<std::string>& specialProcSubstrs);
	std::vector<std::string> GetSpecialProcSubstrs();
	std::vector<ProcessInfo> GetProcessList();
	void SetSpecialProcSubstrs(const std::vector<std::string>& specialProcSubstrs);
	bool EnableDebugPrivileges();
	void Update();

private:
	bool checkSpecialProcSubstrs(const std::string& processName, const std::string& description);
	static bool ProcessReader::SetPrivilege(HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege);
	static bool GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough);
	static std::string GetProcessDescription(DWORD th32ProcessID);
};