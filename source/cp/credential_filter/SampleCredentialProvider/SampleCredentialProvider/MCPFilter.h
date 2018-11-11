
#include <credentialprovider.h>
#include <windows.h>
#include <strsafe.h>
#include <unknwn.h>
#include "common.h"
#include <shlwapi.h>

HRESULT CFilter_CreateInstance(__in REFIID riid, __deref_out void** ppv);

class MCPFilter : public ICredentialProviderFilter
{
public:
	// IUnknown
	IFACEMETHODIMP_(ULONG) AddRef()
	{
		return ++_cRef;
	}

	IFACEMETHODIMP_(ULONG) Release()
	{
		LONG cRef = --_cRef;
		if (!cRef)
		{
			delete this;
		}
		return cRef;
	}

	//这儿返回ok才会调用Filter函数
	IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void** ppv)
	{
		HRESULT hr;
		if ((IID_IUnknown == riid )||
			(IID_ICredentialProviderFilter == riid)){//
			*ppv = this;
			reinterpret_cast<IUnknown*>(*ppv)->AddRef();//
			hr = S_OK;
		}
		/*else if (IID_ICredentialProviderFilter == riid){
			*ppv = static_cast<ICredentialProviderFilter*>(this);
			reinterpret_cast<IUnknown*>(*ppv)->AddRef();
			hr = S_OK;
		}*/
		else{
			*ppv = NULL;
			hr = E_NOINTERFACE;
		}
		return hr;
	}

public:
	IFACEMETHODIMP Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders);
	IFACEMETHODIMP UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsOut);

	friend HRESULT CFilter_CreateInstance(__in REFIID riid, __deref_out void** ppv);
protected:

	MCPFilter();
	__override ~MCPFilter();

private:
	LONG                _cRef;
};
