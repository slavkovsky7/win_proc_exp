#include "stdafx.h"
#include "EventSink.h"
#include "StringUtils.h"
#define _WIN32_DCOM
#pragma comment(lib, "wbemuuid.lib")

ULONG EventSink::AddRef()
{
	return InterlockedIncrement(&lRef);
}

ULONG EventSink::Release() {
	LONG lRef = InterlockedDecrement(&lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT EventSink::QueryInterface(REFIID riid, void** ppv) {
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
		*ppv = (IWbemObjectSink *) this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else return E_NOINTERFACE;
}


HRESULT EventSink::Indicate(long lObjectCount, IWbemClassObject **apObjArray) {
	HRESULT hr = S_OK;
	for (int i = 0; i < lObjectCount; i++)
	{
		_variant_t vtProp;
		hr = apObjArray[i]->Get(L"TargetInstance", 0, &vtProp, 0, 0);
		if (!FAILED(hr)) {
			IUnknown * vtPropUnknown = vtProp;
			CComPtr<IWbemClassObject> pclsObj;

			hr = vtPropUnknown->QueryInterface(IID_IWbemClassObject, reinterpret_cast< void** >(&pclsObj));
			if (SUCCEEDED(hr)) {
				_variant_t var;
				hr = pclsObj->Get(L"ProcessId", 0, &var, NULL, NULL);
				int pid = 0;
				if (SUCCEEDED(hr)) {
					pid = var.intVal;
				}
				VariantClear(&var);

				hr = pclsObj->Get(L"Caption", 0, &var, NULL, NULL);
				std::string procName;
				if (SUCCEEDED(hr)) {
					procName = StringUtils::ConvertBSTRToMBS(var.bstrVal);
				}
				VariantClear(&var);

				if (pid > 0 && !procName.empty()) {
					createProcessCallback(procName, pid);
				}
			}
			vtPropUnknown->Release();
		}
		VariantClear(&vtProp);
	}

	return WBEM_S_NO_ERROR;
}

HRESULT EventSink::SetStatus( LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject __RPC_FAR *pObjParam ) {
	return WBEM_S_NO_ERROR;
}

