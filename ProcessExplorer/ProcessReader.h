#pragma once
#include <mutex>
#include <vector>
#include <string>
#include <map>
#include "EventSink.h"

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
	void SetSpecial(bool val);
	bool operator < (const ProcessInfo& other);
};


typedef std::function< void(void) > TProcessListUpdatedCallback;


class ProcessReader {
private:
	std::map<int, ProcessInfo> processMap;
	std::vector<std::string> specialProcSubstrs;
	std::mutex mutex;
	TProcessListUpdatedCallback processListUpdatedCallback;
public:
	ProcessReader(const std::vector<std::string>& specialProcSubstrs, TProcessListUpdatedCallback processListUpdatedCallback);
	std::vector<std::string> GetSpecialProcSubstrs();
	std::vector<ProcessInfo> GetProcessList();
	void CreateProcessMap();
	void SetSpecialProcSubstrs(const std::vector<std::string>& specialProcSubstrs);
	bool EnableDebugPrivileges();
private:
	bool CheckSpecialProcSubstrs(const std::string& processName, const std::string& description);
	void OnProcessCreate(const std::string& str, int pid);
	void OnProcessTerminate(const std::string& str, int pid);
public:
	HRESULT RegisterCallBack(TProcessNotification createProcessCallback, TProcessNotification terminateProcessCallback);
	HRESULT RegisterEventSyncQuery(CComPtr<IWbemServices> pSvc, const std::string& query, CComPtr<EventSink> eventSync);
private:
	static bool ProcessReader::SetPrivilege(HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege);
	static bool GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough);
	static std::string GetProcessDescription(DWORD th32ProcessID);

};
