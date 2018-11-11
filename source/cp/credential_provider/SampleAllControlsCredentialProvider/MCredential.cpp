//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
//
//

#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif
#include <unknwn.h>
#include "MCredential.h"
#include "DriverOperation.h"
#include "DataBaseOperation.h"
#include "guid.h"

extern HINSTANCE g_hinst;

BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL ShowDlg();

bool RegisterIt(HWND hDlg);

// CSampleCredential ////////////////////////////////////////////////////////

MCredential::MCredential():
	_cRef(1),
	_pCredProvCredentialEvents(NULL)
{
	DllAddRef();

	ZeroMemory(_rgCredProvFieldDescriptors, sizeof(_rgCredProvFieldDescriptors));
	ZeroMemory(_rgFieldStatePairs, sizeof(_rgFieldStatePairs));
	ZeroMemory(_rgFieldStrings, sizeof(_rgFieldStrings));
	_bChecked = FALSE;
	_dwComboIndex = 0;
}

MCredential::~MCredential()
{
	if (_rgFieldStrings[SFI_PASSWORD])
	{
		// CoTaskMemFree (below) deals with NULL, but StringCchLength does not.
		size_t lenPassword;
		HRESULT hr = StringCchLengthW(_rgFieldStrings[SFI_PASSWORD], 128, &(lenPassword));
		if (SUCCEEDED(hr))
		{
			SecureZeroMemory(_rgFieldStrings[SFI_PASSWORD], lenPassword * sizeof(*_rgFieldStrings[SFI_PASSWORD]));
		}
		else
		{
			// TODO: Determine how to handle count error here.
		}
	}
	for (int i = 0; i < ARRAYSIZE(_rgFieldStrings); i++)
	{
		CoTaskMemFree(_rgFieldStrings[i]);
		CoTaskMemFree(_rgCredProvFieldDescriptors[i].pszLabel);
	}

	DllRelease();
}


//设置凭据域显示信息
HRESULT MCredential::Initialize(
	CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
	const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
	const FIELD_STATE_PAIR* rgfsp
	)
{
	HRESULT hr = S_OK;

	_cpus = cpus;

	// Copy the field descriptors for each field. This is useful if you want to vary the field
	// descriptors based on what Usage scenario the credential was created for.
	for (DWORD i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(_rgCredProvFieldDescriptors); i++)
	{
		_rgFieldStatePairs[i] = rgfsp[i];
		hr = FieldDescriptorCopy(rgcpfd[i], &_rgCredProvFieldDescriptors[i]);
	}

	// Initialize the String value of all the fields. 
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Owl", &_rgFieldStrings[SFI_LARGE_TEXT]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"welcome", &_rgFieldStrings[SFI_SMALL_TEXT]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"", &_rgFieldStrings[SFI_EDIT_TEXT]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"", &_rgFieldStrings[SFI_PASSWORD]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Submit", &_rgFieldStrings[SFI_SUBMIT_BUTTON]);
	}
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Introduction", &_rgFieldStrings[SFI_COMMAND_LINK]);
	}

	return S_OK;
}

// LogonUI 回调
HRESULT MCredential::Advise(
	ICredentialProviderCredentialEvents* pcpce
	)
{
	if (_pCredProvCredentialEvents != NULL)
	{
		_pCredProvCredentialEvents->Release();
	}
	_pCredProvCredentialEvents = pcpce;
	_pCredProvCredentialEvents->AddRef();
	return S_OK;
}

// LogonUI 释放回调
HRESULT MCredential::UnAdvise()
{
	if (_pCredProvCredentialEvents)
	{
		_pCredProvCredentialEvents->Release();
	}
	_pCredProvCredentialEvents = NULL;
	return S_OK;
}

// LogonUI 调用,凭据被选中的操作
HRESULT MCredential::SetSelected(BOOL* pbAutoLogon)
{
	*pbAutoLogon = FALSE;  
	return S_OK;
}

//LogonUI 清空密码框
HRESULT MCredential::SetDeselected()
{
	HRESULT hr = S_OK;
	if (_rgFieldStrings[SFI_PASSWORD])
	{
		// CoTaskMemFree (below) deals with NULL, but StringCchLength does not.
		size_t lenPassword;
		hr = StringCchLengthW(_rgFieldStrings[SFI_PASSWORD], 128, &(lenPassword));
		if (SUCCEEDED(hr))
		{
			SecureZeroMemory(_rgFieldStrings[SFI_PASSWORD], lenPassword * sizeof(*_rgFieldStrings[SFI_PASSWORD]));

			CoTaskMemFree(_rgFieldStrings[SFI_PASSWORD]);
			hr = SHStrDupW(L"", &_rgFieldStrings[SFI_PASSWORD]);
		}

		if (SUCCEEDED(hr) && _pCredProvCredentialEvents)
		{
			_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, _rgFieldStrings[SFI_PASSWORD]);
		}
	}

	return hr;
}

//logonUI 获得信息显示tile
HRESULT MCredential::GetFieldState(
	DWORD dwFieldID,
	CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
	CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis
	)
{
	HRESULT hr;
	
	// Validate our parameters.
	if ((dwFieldID < ARRAYSIZE(_rgFieldStatePairs)) && pcpfs && pcpfis)
	{
		*pcpfs = _rgFieldStatePairs[dwFieldID].cpfs;
		*pcpfis = _rgFieldStatePairs[dwFieldID].cpfis;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}
	return hr;
}

// 将 ppwsz 转换为在dwFieldID域中的 string
HRESULT MCredential::GetStringValue(
	DWORD dwFieldID, 
	PWSTR* ppwsz
	)
{
	HRESULT hr;

	// Check to make sure dwFieldID is a legitimate index
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && ppwsz) 
	{
		// Make a copy of the string and return that. The caller
		// is responsible for freeing it.
		hr = SHStrDupW(_rgFieldStrings[dwFieldID], ppwsz);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// 获得需要显示的图片
HRESULT MCredential::GetBitmapValue(
	DWORD dwFieldID, 
	HBITMAP* phbmp
	)
{
	HRESULT hr;
	if ((SFI_TILEIMAGE == dwFieldID) && phbmp)
	{
		HBITMAP hbmp = LoadBitmap(HINST_THISDLL, MAKEINTRESOURCE(IDB_TILE_IMAGE));
		if (hbmp != NULL)
		{
			hr = S_OK;
			*phbmp = hbmp;
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//获得提交按钮的值
HRESULT MCredential::GetSubmitButtonValue(
	DWORD dwFieldID,
	DWORD* pdwAdjacentTo
	)
{
	HRESULT hr;

	if (SFI_SUBMIT_BUTTON == dwFieldID && pdwAdjacentTo)
	{
		// pdwAdjacentTo is a pointer to the fieldID you want the submit button to 
		// appear next to.
		*pdwAdjacentTo = SFI_PASSWORD;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}
	return hr;
}

//设置可接收String控件的值
HRESULT MCredential::SetStringValue(
	DWORD dwFieldID, 
	PCWSTR pwz      
	)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && 
		(CPFT_EDIT_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft || 
		CPFT_PASSWORD_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft)) 
	{
		PWSTR* ppwszStored = &_rgFieldStrings[dwFieldID];
		CoTaskMemFree(*ppwszStored);
		hr = SHStrDupW(pwz, ppwszStored);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//获得用户输入的数据(用户名，密码，电话号，邮箱)
//并且打开数据库进行注册操作。
bool RegisterIt(HWND hDlg){
	//获得每个编辑控件的中用户输入的值
	TCHAR UserName[255], PassWord[255], PhoneNumber[255], Email[255];

	GetDlgItemText(hDlg, IDC_EDIT1, UserName, 255);
	GetDlgItemText(hDlg, IDC_EDIT2, PassWord, 255);
	GetDlgItemText(hDlg, IDC_EDIT3, PhoneNumber, 255);
	GetDlgItemText(hDlg, IDC_EDIT4, Email, 255);

	//将获得的用户输入的数据用MessageBox显示出来
	if (UserName != NULL)
		MessageBox(hDlg, UserName, "UserName", 0);
	if (PassWord != NULL)
		MessageBox(hDlg, PassWord, "PassWord", 0);
	if (PhoneNumber != NULL)
		MessageBox(hDlg, PhoneNumber, "PhoneNumber", 0);
	if (Email != NULL)
		MessageBox(hDlg, Email, "Email", 0);

//	DataBaseOperation *dbo = new DataBaseOperation();
	//打开数据库进行注册操作
	/*if (dbo->OpenDB())
		return true;
	else
		return false;*/
	return true;
}

//对话框回调函数
BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return (TRUE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return (TRUE);
		case IDOK://尝试两次注册数据库
			int i = 0;
			while (i<2){
				if (RegisterIt(hDlg)){
					MessageBoxA(hDlg, "U push OK !", "Information", 0);
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					return (TRUE);
				}
				i++;
			}
			return (FALSE);
		}
		return (FALSE);
	}
	return (FALSE);
}

//点击SignIn打开并显示模态对话框
BOOL ShowDlg()
{
	INT_PTR hwndDLG = DialogBox(g_hinst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc);
	if (hwndDLG == -1)
		return FALSE;
	return TRUE;
}

BOOL ShowIntroduction(){
	return MessageBoxA(NULL,">>>Introduction>>> \n This software made by VFly Team.","Infomation",MB_OK);
}

// 当用户点击注册调用
HRESULT MCredential::CommandLinkClicked(DWORD dwFieldID)
{
	HRESULT hr;
	// Validate parameter.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && 
		(CPFT_COMMAND_LINK == _rgCredProvFieldDescriptors[dwFieldID].cpft))
	{
		//if (!ShowDlg()) MessageBox(NULL,"Open Register Dialog Failed !!!","ERROR",0);
		if (ShowIntroduction())
			hr = S_OK;
	}
	else
		hr = E_INVALIDARG;

	return hr;
}

//获得并提交用户名和密码
HRESULT MCredential::GetSerialization(
	CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
	CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
	PWSTR* ppwszOptionalStatusText, 
	CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
	)
{
	UNREFERENCED_PARAMETER(ppwszOptionalStatusText);
	UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);

	HRESULT hr;

	WCHAR wsz[MAX_COMPUTERNAME_LENGTH+1];
	DWORD cch = ARRAYSIZE(wsz);
	if (GetComputerNameW(wsz, &cch))
	{
		PWSTR pwzProtectedPassword;

		//密码复制至pwzProtectedPassword
		hr = ProtectIfNecessaryAndCopyPassword(_rgFieldStrings[SFI_PASSWORD], _cpus, &pwzProtectedPassword);

		if (SUCCEEDED(hr))
		{
			KERB_INTERACTIVE_UNLOCK_LOGON kiul;

			//将Password和UserName封装进Kiul并返回.
			hr = KerbInteractiveUnlockLogonInit(wsz, _rgFieldStrings[SFI_EDIT_TEXT], pwzProtectedPassword, _cpus, &kiul);

			if (SUCCEEDED(hr))
			{
				//账户验证
				//当用户输入用户名为Yue时，禁止登陆，输入非Yue时同时密码为123，则进行正确登陆操作
				hr = KerbInteractiveUnlockLogonPack(kiul, &pcpcs->rgbSerialization, &pcpcs->cbSerialization);

				if (SUCCEEDED(hr))
				{
					ULONG ulAuthPackage;
					hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);
					if (SUCCEEDED(hr))
					{
						pcpcs->ulAuthenticationPackage = ulAuthPackage;
						pcpcs->clsidCredentialProvider = CLSID_CProvider;
						*pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
					}
				}
			}

			CoTaskMemFree(pwzProtectedPassword);
		}
	}
	else
	{
		DWORD dwErr = GetLastError();
		hr = HRESULT_FROM_WIN32(dwErr);
	}

	return hr;
}

struct REPORT_RESULT_STATUS_INFO
{
	NTSTATUS ntsStatus;
	NTSTATUS ntsSubstatus;
	PWSTR     pwzMessage;
	CREDENTIAL_PROVIDER_STATUS_ICON cpsi;
};

static const REPORT_RESULT_STATUS_INFO s_rgLogonStatusInfo[] =
{
	{ STATUS_LOGON_FAILURE, STATUS_SUCCESS, L"Incorrect password or username.", CPSI_ERROR, },
	{ STATUS_ACCOUNT_RESTRICTION, STATUS_ACCOUNT_DISABLED, L"The account is disabled.", CPSI_WARNING },
};

//可选项
HRESULT MCredential::ReportResult(
	NTSTATUS ntsStatus, 
	NTSTATUS ntsSubstatus,
	PWSTR* ppwszOptionalStatusText, 
	CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
	)
{
	*ppwszOptionalStatusText = NULL;
	*pcpsiOptionalStatusIcon = CPSI_NONE;

	DWORD dwStatusInfo = (DWORD)-1;

	// Look for a match on status and substatus.
	for (DWORD i = 0; i < ARRAYSIZE(s_rgLogonStatusInfo); i++)
	{
		if (s_rgLogonStatusInfo[i].ntsStatus == ntsStatus && s_rgLogonStatusInfo[i].ntsSubstatus == ntsSubstatus)
		{
			dwStatusInfo = i;
			break;
		}
	}

	if ((DWORD)-1 != dwStatusInfo)
	{
		if (SUCCEEDED(SHStrDupW(s_rgLogonStatusInfo[dwStatusInfo].pwzMessage, ppwszOptionalStatusText)))
		{
			*pcpsiOptionalStatusIcon = s_rgLogonStatusInfo[dwStatusInfo].cpsi;
		}
	}

	// If we failed the logon, try to erase the password field.
	if (!SUCCEEDED(HRESULT_FROM_NT(ntsStatus)))
	{
		if (_pCredProvCredentialEvents)
		{
			_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, L"");
		}
	}

	// Since NULL is a valid value for *ppwszOptionalStatusText and *pcpsiOptionalStatusIcon
	// this function can't fail.
	return S_OK;
}
