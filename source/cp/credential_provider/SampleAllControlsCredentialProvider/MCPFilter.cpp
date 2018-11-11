
#include "MCPFilter.h"
#include "Dll.h"
#include "guid.h"
#include <wincred.h>

CProviderFilter::CProviderFilter(void)
{
}

CProviderFilter::~CProviderFilter(void)
{
}


HRESULT CProviderFilter::Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders)
{
	int i = 0;
	switch (cpus)
	{
	case CPUS_CREDUI:
		if (dwFlags & CREDUIWIN_GENERIC) {
			return S_OK;
		}
	case CPUS_LOGON:
	case CPUS_UNLOCK_WORKSTATION:
		//Filters out the default Windows provider (only for Logon and Unlock scenarios)
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
	case CPUS_CHANGE_PASSWORD:
		
	default:
		return E_INVALIDARG;
	}
}

HRESULT CProviderFilter::UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsOut)
{
	UNREFERENCED_PARAMETER(pcpcsOut);
	UNREFERENCED_PARAMETER(pcpcsIn);
	return E_NOTIMPL;
}


// Boilerplate code to create our provider.
HRESULT CFilter_CreateInstance(__in REFIID riid, __deref_out void** ppv)
{
	HRESULT hr;

	CProviderFilter* pFilter = new CProviderFilter();

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
