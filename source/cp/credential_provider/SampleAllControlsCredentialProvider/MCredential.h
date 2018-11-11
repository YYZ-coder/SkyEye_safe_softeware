
#pragma once

#include <windows.h>
#include <strsafe.h>
#include <shlguid.h>
#include "helpers.h"
#include "dll.h"
#include "resource.h"

class MCredential : public ICredentialProviderCredential
{
public:
	// IUnknown
	STDMETHOD_(ULONG, AddRef)()
	{
		return _cRef++;
	}

	STDMETHOD_(ULONG, Release)()
	{
		LONG cRef = _cRef--;
		if (!cRef)
		{
			delete this;
		}
		return cRef;
	}

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv)
	{
		HRESULT hr;
		if (ppv != NULL)
		{
			if (IID_IUnknown == riid ||
				IID_ICredentialProviderCredential == riid)
			{
				*ppv = static_cast<IUnknown*>(this);
				reinterpret_cast<IUnknown*>(*ppv)->AddRef();
				hr = S_OK;
			}
			else
			{
				*ppv = NULL;
				hr = E_NOINTERFACE;
			}
		}
		else
		{
			hr = E_INVALIDARG;
		}
		return hr;
	}
public:
	// ICredentialProviderCredential
	IFACEMETHODIMP Advise(ICredentialProviderCredentialEvents* pcpce);
	IFACEMETHODIMP UnAdvise();

	IFACEMETHODIMP SetSelected(BOOL* pbAutoLogon);
	IFACEMETHODIMP SetDeselected();

	IFACEMETHODIMP GetFieldState(DWORD dwFieldID,
		CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
		CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);

	IFACEMETHODIMP GetStringValue(DWORD dwFieldID, PWSTR* ppwsz);
	IFACEMETHODIMP GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp);
	IFACEMETHODIMP GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, PWSTR* ppwszLabel){
		UNREFERENCED_PARAMETER(dwFieldID);
		UNREFERENCED_PARAMETER(pbChecked);
		UNREFERENCED_PARAMETER(ppwszLabel);
		return E_NOTIMPL;
	}
	IFACEMETHODIMP GetComboBoxValueCount(DWORD dwFieldID, DWORD* pcItems, DWORD* pdwSelectedItem){
		UNREFERENCED_PARAMETER(dwFieldID);
		UNREFERENCED_PARAMETER(pcItems);
		UNREFERENCED_PARAMETER(pdwSelectedItem);
		return E_NOTIMPL;
	}
	IFACEMETHODIMP GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, PWSTR* ppwszItem){
		UNREFERENCED_PARAMETER(dwFieldID);
		UNREFERENCED_PARAMETER(dwItem);
		UNREFERENCED_PARAMETER(ppwszItem);
		return E_NOTIMPL;
	}
	IFACEMETHODIMP GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo);

	IFACEMETHODIMP SetStringValue(DWORD dwFieldID, PCWSTR pwz);
	IFACEMETHODIMP SetCheckboxValue(DWORD dwFieldID, BOOL bChecked){
		UNREFERENCED_PARAMETER(dwFieldID);
		UNREFERENCED_PARAMETER(bChecked);
		return E_NOTIMPL;
	}
	IFACEMETHODIMP SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem){
		UNREFERENCED_PARAMETER(dwFieldID);
		UNREFERENCED_PARAMETER(dwSelectedItem);
		return E_NOTIMPL;
	}
    IFACEMETHODIMP CommandLinkClicked(DWORD dwFieldID);

    IFACEMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, 
                                    CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
                                    PWSTR* ppwszOptionalStatusText, 
                                    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
    IFACEMETHODIMP ReportResult(NTSTATUS ntsStatus, 
                                NTSTATUS ntsSubstatus,
                                PWSTR* ppwszOptionalStatusText, 
                                CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

  public:
    HRESULT Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
                       const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
                       const FIELD_STATE_PAIR* rgfsp);
	MCredential();

	virtual ~MCredential();

  private:
    LONG                                    _cRef;

    CREDENTIAL_PROVIDER_USAGE_SCENARIO      _cpus; // The usage scenario for which we were enumerated.

    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR    _rgCredProvFieldDescriptors[SFI_NUM_FIELDS];    // An array holding the 
                                                                                            // type and name of each 
                                                                                            // field in the tile.

    FIELD_STATE_PAIR                        _rgFieldStatePairs[SFI_NUM_FIELDS];                // An array holding the 
                                                                                            // state of each field in 
                                                                                            // the tile.

    PWSTR                                    _rgFieldStrings[SFI_NUM_FIELDS];                // An array holding the 
                                                                                            // string value of each 
                                                                                            // field. This is different 
                                                                                            // from the name of the 
                                                                                            // field held in 
                                                                                            // _rgCredProvFieldDescriptors.

    ICredentialProviderCredentialEvents*    _pCredProvCredentialEvents;                        // Used to update fields.
    BOOL                                    _bChecked;                                        // Tracks the state of our 
                                                                                            // checkbox.

    DWORD                                    _dwComboIndex;                                    // Tracks the current index 
                                                                                            // of our combobox.

};
