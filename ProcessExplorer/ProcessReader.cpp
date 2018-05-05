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

void ProcessInfo::SetSpecial(bool val) {
	this->special = val;
}

bool ProcessInfo::operator < (const ProcessInfo& other) {
	return (StringUtils::Tolower(processName).compare(StringUtils::Tolower(other.processName)) < 0);
}

void ProcessReader::OnProcessCreate(const std::string& procName, int pid) {
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (processMap.count(pid) == 0) {
			std::string desc = GetProcessDescription(pid);
			bool special = CheckSpecialProcSubstrs(procName, desc);
			ProcessInfo processInfo(pid, procName, desc, special);
			processMap.insert(std::make_pair(pid, processInfo));
		}
	}
	processListUpdatedCallback();
}

void ProcessReader::OnProcessTerminate(const std::string& str, int pid) {
	{
		std::lock_guard<std::mutex> lock(mutex);
		processMap.erase(pid);
	}
	processListUpdatedCallback();
}


ProcessReader::ProcessReader(const std::vector<std::string>& specialProcSubstrs, TProcessListUpdatedCallback processListUpdatedCallback) :
	specialProcSubstrs(specialProcSubstrs),
	processListUpdatedCallback(processListUpdatedCallback)
{
	TProcessNotification onProcessCreate = std::bind(&ProcessReader::OnProcessCreate, this, std::placeholders::_1, std::placeholders::_2);
	TProcessNotification onProcessTerminate = std::bind(&ProcessReader::OnProcessTerminate, this, std::placeholders::_1, std::placeholders::_2);
	RegisterCallBack(onProcessCreate, onProcessTerminate);
}

std::vector<std::string> ProcessReader::GetSpecialProcSubstrs() {
	std::lock_guard<std::mutex> lock(mutex);
	return specialProcSubstrs;
}

void ProcessReader::SetSpecialProcSubstrs(const std::vector<std::string>& specialProcSubstrs) {
	std::lock_guard<std::mutex> lock(mutex);
	this->specialProcSubstrs = specialProcSubstrs;
	for (auto& kv : this->processMap) {
		ProcessInfo* procInfo = &kv.second;
		procInfo->SetSpecial(CheckSpecialProcSubstrs(procInfo->GetProcessName(), procInfo->GetProcessDescription()));
	}
}

bool ProcessReader::EnableDebugPrivileges() {
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

			HANDLE procToken;
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &procToken)) {
				return false;
			}

			if (!SetPrivilege(procToken, SE_DEBUG_NAME, TRUE)) {
				return false;
			}
			CloseHandle(procToken);

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
	std::vector<ProcessInfo> result;
	for (auto& kv : this->processMap) {
		result.push_back(kv.second);
	}
	std::sort(result.begin(), result.end());
	return result;
}

void ProcessReader::CreateProcessMap() {
	std::lock_guard<std::mutex> lock(mutex);
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap != INVALID_HANDLE_VALUE) {
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hProcessSnap, &pe32)) {
			do {
				std::string processName = std::string(pe32.szExeFile);
				std::string description = GetProcessDescription(pe32.th32ProcessID);
				bool special = CheckSpecialProcSubstrs(processName, description);
				this->processMap.insert(std::make_pair(pe32.th32ProcessID, ProcessInfo(pe32.th32ProcessID, processName, description, special)));

			} while (Process32Next(hProcessSnap, &pe32));
		}
		CloseHandle(hProcessSnap);
	}
}


bool ProcessReader::CheckSpecialProcSubstrs(const std::string& processName, const std::string& description) {
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


HRESULT ProcessReader::RegisterCallBack(TProcessNotification createProcessCallback, TProcessNotification terminateProcessCallback) {
	HRESULT hres;
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) {
		return hres;
	}

	hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (FAILED(hres)) {
		CoUninitialize();
		return hres;
	}

	CComPtr<IWbemLocator> pLoc;
	hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
	if (FAILED(hres)) {
		CoUninitialize();
		return hres;
	}

	CComPtr<IWbemServices> pSvc;
	hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),NULL, NULL,  0, NULL, 0, 0, &pSvc);
	if (FAILED(hres)) {
		CoUninitialize();
		return hres;
	}

	hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,  RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if (FAILED(hres)) {
		CoUninitialize();
		return hres;
	}

	CComPtr<EventSink> eventSyncCreate(new EventSink(createProcessCallback));
	hres = RegisterEventSyncQuery(pSvc, "SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'", eventSyncCreate);

	CComPtr<EventSink> eventSyncTerminate(new EventSink(terminateProcessCallback));
	hres = RegisterEventSyncQuery(pSvc, "SELECT * FROM __InstanceDeletionEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'", eventSyncTerminate);
	
	if (FAILED(hres)) {
		CoUninitialize();
		return hres;
	}
	return hres;
}


HRESULT ProcessReader::RegisterEventSyncQuery(CComPtr<IWbemServices> pSvc, const std::string& query, CComPtr<EventSink> eventSync) {

	CComPtr<IUnsecuredApartment> pUnsecApp;
	HRESULT hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL, CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment, (void**)&pUnsecApp);


	CComPtr<IUnknown> pStubUnk = NULL;
	pUnsecApp->CreateObjectStub(eventSync, &pStubUnk);

	CComPtr<IWbemObjectSink> pStubSink = NULL;
	pStubUnk->QueryInterface(IID_IWbemObjectSink, (void **)&pStubSink);

	hres = pSvc->ExecNotificationQueryAsync(
		_bstr_t("WQL"),
		_bstr_t(query.c_str()),
		WBEM_FLAG_SEND_STATUS,
		NULL,
		pStubSink);

	return hres;
}