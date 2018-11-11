
#include <credentialprovider.h>
#include "MProvider.h"
#include "MCredential.h"
#include "guid.h"

// CSampleProvider ////////////////////////////////////////////////////////

MProvider::MProvider() :
    _cRef(1)
{
    DllAddRef();

    _pCredential = NULL;
}

MProvider::~MProvider()
{
    if (_pCredential != NULL)
    {
        _pCredential->Release();
        _pCredential = NULL;
    }

    DllRelease();
}

//设置使用的Scenario
HRESULT MProvider::SetUsageScenario(
    CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    DWORD dwFlags
    )
{
    UNREFERENCED_PARAMETER(dwFlags);
    HRESULT hr;

    // Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
    // that we're not designed for that scenario.
    switch (cpus)
    {
    case CPUS_LOGON:
    case CPUS_UNLOCK_WORKSTATION:       
        _cpus = cpus;

        // Create and initialize our credential.
        // A more advanced credprov might only enumerate tiles for the user whose owns the locked
        // session, since those are the only creds that wil work
		_pCredential = new MCredential();
        if (_pCredential != NULL)
        {
            hr = _pCredential->Initialize(_cpus, s_rgCredProvFieldDescriptors, s_rgFieldStatePairs);
            if (FAILED(hr))
            {
                _pCredential->Release();
                _pCredential = NULL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        break;

    case CPUS_CHANGE_PASSWORD:
    case CPUS_CREDUI:
        hr = E_NOTIMPL;
        break;

    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

STDMETHODIMP MProvider::SetSerialization(
    const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs
    )
{
    UNREFERENCED_PARAMETER(pcpcs);
    return E_NOTIMPL;
}

//LogonUI 回调
HRESULT MProvider::Advise(
    ICredentialProviderEvents* pcpe,
    UINT_PTR upAdviseContext
    )
{
    UNREFERENCED_PARAMETER(pcpe);
    UNREFERENCED_PARAMETER(upAdviseContext);

    return E_NOTIMPL;
}

//LogonUI回调当 ICredentialProviderEvents回调无效
HRESULT MProvider::UnAdvise()
{
    return E_NOTIMPL;
}

//LogonUI 调用获得当前域元素数量
HRESULT MProvider::GetFieldDescriptorCount(
    DWORD* pdwCount
    )
{
    *pdwCount = SFI_NUM_FIELDS;
    return S_OK;
}

// 获得域描述
HRESULT MProvider::GetFieldDescriptorAt(
    DWORD dwIndex, 
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
    )
{    
    HRESULT hr;

    // Verify dwIndex is a valid field.
    if ((dwIndex < SFI_NUM_FIELDS) && ppcpfd)
    {
        hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
    }
    else
    { 
        hr = E_INVALIDARG;
    }

    return hr;
}

//获得凭据数量
HRESULT MProvider::GetCredentialCount(
    DWORD* pdwCount,
    DWORD* pdwDefault,
    BOOL* pbAutoLogonWithDefault
    )
{
    *pdwCount = 1;
    *pdwDefault = 0;
    *pbAutoLogonWithDefault = FALSE;
    return S_OK;
}

//返回凭据
HRESULT MProvider::GetCredentialAt(
    DWORD dwIndex, 
    ICredentialProviderCredential** ppcpc
    )
{
    HRESULT hr;
    if((dwIndex == 0) && ppcpc)
    {
        hr = _pCredential->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

//实例化Provider
HRESULT MProvider_CreateInstance(REFIID riid, void** ppv)
{
    HRESULT hr;

    MProvider* pProvider = new MProvider();

    if (pProvider)
    {
        hr = pProvider->QueryInterface(riid, ppv);
        pProvider->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}
