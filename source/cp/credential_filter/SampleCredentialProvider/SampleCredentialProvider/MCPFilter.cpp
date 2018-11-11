
#include "MCPFilter.h"
#include "Dll.h"
#include "guid.h"
#include <wincred.h> 

MCPFilter::MCPFilter(void)
{
}

MCPFilter::~MCPFilter(void)
{
}

HRESULT MCPFilter::Filter(
	CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, 
	DWORD dwFlags, GUID* rgclsidProviders, 
	BOOL* rgbAllow, DWORD cProviders)
{
	UNREFERENCED_PARAMETER(dwFlags);
	MessageBoxA(NULL,"Filtered","Info",0);
	int i = 0;
	switch (cpus)
	{
	case CPUS_CREDUI:
	case CPUS_LOGON:
	case CPUS_UNLOCK_WORKSTATION:
		
		/*for (int i = 0; i < cProviders; i++){
			if (IsEqualGUID(rgclsidProviders[i], CLSID_CProvider))
				rgbAllow[i] = TRUE;
			else if (IsEqualGUID(rgclsidProviders[i], CLSID_PasswordCredentialProvider))
				rgbAllow[i] = FALSE;
			else
				rgbAllow[i] = FALSE;
			break;
		}*/

		for (i = 0; i < (int)cProviders; i++)
		{
			if (IsEqualGUID(rgclsidProviders[i], CLSID_CProvider))
				break;
		}
		if (i != (int)cProviders)
		{
			int j = 0;
			for (j = 0; j < (int)cProviders; j++)
				rgbAllow[j] = FALSE;
			rgbAllow[i] = TRUE;
		}
		return S_OK;
		break;
	case CPUS_CHANGE_PASSWORD:
		return E_NOTIMPL;
	default:
		return E_INVALIDARG;
	}
}

HRESULT MCPFilter::UpdateRemoteCredential(
	const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn,
	CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsOut)

{
	UNREFERENCED_PARAMETER(pcpcsOut);
	UNREFERENCED_PARAMETER(pcpcsIn);
	return E_NOTIMPL;
}

HRESULT CFilter_CreateInstance(__in REFIID riid, __deref_out void** ppv)
{
	HRESULT hr;

	MCPFilter* pFilter = new MCPFilter();

	if (pFilter)
	{
		hr = pFilter->QueryInterface(riid, ppv);
		// TODO: this release is probably needed to not leak memory, but right now we crash with it.
		//pFilter->Release();
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}
