#pragma once
#include <functional>
#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

typedef std::function< void(const std::string&, int) > TProcessNotification;

class EventSink : public IWbemObjectSink
{
private:
	LONG lRef;

	TProcessNotification createProcessCallback;

	//typedef std::function< void(const std::string&, int) > TTerminateProcessCallback;

public:
	EventSink(TProcessNotification createProcessCallback)
		:lRef(0),
		createProcessCallback(createProcessCallback)
	{}


	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
	virtual HRESULT STDMETHODCALLTYPE Indicate(long lObjectCount, IWbemClassObject **apObjArray);
	virtual HRESULT STDMETHODCALLTYPE SetStatus(LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject __RPC_FAR *pObjParam);
};