
#include "helpers.h"
#include <intsafe.h>
#include <wincred.h>
#include "DriverOperation.h"
#include "DataBaseOperation.h"
#include <iostream>
using namespace std;


HRESULT FieldDescriptorCoAllocCopy(
								   const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR& rcpfd,
								   CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
								   )
{
	HRESULT hr;
	DWORD cbStruct = sizeof(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR);

	CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* pcpfd = 
		(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR*)CoTaskMemAlloc(cbStruct);

	if (pcpfd)
	{
		pcpfd->dwFieldID = rcpfd.dwFieldID;
		pcpfd->cpft = rcpfd.cpft;

		if (rcpfd.pszLabel)
		{
			hr = SHStrDupW(rcpfd.pszLabel, &pcpfd->pszLabel);
		}
		else
		{
			pcpfd->pszLabel = NULL;
			hr = S_OK;
		}
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}
	if (SUCCEEDED(hr))
	{
		*ppcpfd = pcpfd;
	}
	else
	{
		CoTaskMemFree(pcpfd);  
		*ppcpfd = NULL;
	}


	return hr;
}


HRESULT FieldDescriptorCopy(
							const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR& rcpfd,
							CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* pcpfd
							)
{
	HRESULT hr;
	CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR cpfd;

	cpfd.dwFieldID = rcpfd.dwFieldID;
	cpfd.cpft = rcpfd.cpft;

	if (rcpfd.pszLabel)
	{
		hr = SHStrDupW(rcpfd.pszLabel, &cpfd.pszLabel);
	}
	else
	{
		cpfd.pszLabel = NULL;
		hr = S_OK;
	}

	if (SUCCEEDED(hr))
	{
		*pcpfd = cpfd;
	}

	return hr;
}


HRESULT UnicodeStringInitWithString(
									   PWSTR pwz,
									   UNICODE_STRING* pus
									   )
{
	HRESULT hr;
	if (pwz)
	{
		size_t lenString;
		hr = StringCchLengthW(pwz, USHORT_MAX, &(lenString));

		if (SUCCEEDED(hr))
		{
			USHORT usCharCount;
			hr = SizeTToUShort(lenString, &usCharCount);
			if (SUCCEEDED(hr))
			{
				USHORT usSize;
				hr = SizeTToUShort(sizeof(WCHAR), &usSize);
				if (SUCCEEDED(hr))
				{
					hr = UShortMult(usCharCount, usSize, &(pus->Length)); // Explicitly NOT including NULL terminator
					if (SUCCEEDED(hr))
					{
						pus->MaximumLength = pus->Length;
						pus->Buffer = pwz;
						hr = S_OK;
					}
					else
					{
						hr = HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
					}
				}
			}
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}
	return hr;
}


static void _UnicodeStringPackedUnicodeStringCopy(
	const UNICODE_STRING& rus,
	PWSTR pwzBuffer,
	UNICODE_STRING* pus
	)
{
	pus->Length = rus.Length;
	pus->MaximumLength = rus.Length;
	pus->Buffer = pwzBuffer;

	CopyMemory(pus->Buffer, rus.Buffer, pus->Length);
}

//wchar_t TO char
char *w2c(char *pcstr, const wchar_t *pwstr, size_t len)
{
	int nlength = wcslen(pwstr);

	//获取转换后的长度

	unsigned int nbytes = WideCharToMultiByte(0, // specify the code page used to perform the conversion
		0,         // no special flags to handle unmapped characters
		pwstr,     // wide character string to convert
		nlength,   // the number of wide characters in that string
		NULL,      // no output buffer given, we just want to know how long it needs to be
		0,
		NULL,      // no replacement character given
		NULL);    // we don't want to know if a character didn't make it through the translation
	// make sure the buffer is big enough for this, making it larger if necessary
	if (nbytes > len)   nbytes = len;
	// 通过以上得到的结果，转换unicode 字符为ascii 字符
	WideCharToMultiByte(0, // specify the code page used to perform the conversion
		0,         // no special flags to handle unmapped characters
		pwstr,   // wide character string to convert
		nlength,   // the number of wide characters in that string
		pcstr, // put the output ascii characters at the end of the buffer
		nbytes,                           // there is at least this much space there
		NULL,      // no replacement character given
		NULL);

	return pcstr;
}

//char TO wchar_t
wchar_t *c2w(char *szStr){
	int nLen = MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,szStr,-1,NULL,0);
	if (nLen == 0)
		return NULL;
	wchar_t *pResult = new wchar_t[nLen];
	MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,szStr,-1,pResult,nLen);
	return pResult;
}

HRESULT KerbInteractiveUnlockLogonInit(
									   PWSTR pwzDomain,
									   PWSTR pwzUsername,
									   PWSTR pwzPassword,
									   CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
									   KERB_INTERACTIVE_UNLOCK_LOGON* pkiul
									   )
{
	KERB_INTERACTIVE_UNLOCK_LOGON kiul;
	ZeroMemory(&kiul, sizeof(kiul));

	//获得新建的KERB_INTERACTIVE_UNLOCK_LOGON的Logon的引用
	KERB_INTERACTIVE_LOGON* pkil = &kiul.Logon;

	// Initialize the UNICODE_STRINGS to share our username and password strings.
	HRESULT hr = UnicodeStringInitWithString(pwzDomain, &pkil->LogonDomainName);

	if (SUCCEEDED(hr))
	{
		
		///输入的用户名为Yue时，将用户名设置为123，而当用户名不为Yue的时候，设置用户名为Yue
		hr = UnicodeStringInitWithString(pwzUsername, &pkil->UserName);

		//将用户输入的用户名字符串拷贝进pkil->Username中
		//hr = UnicodeStringInitWithString(pwzUsername, &pkil->UserName);

		if (SUCCEEDED(hr))
		{
			if (SUCCEEDED(hr))
			{
				/////////////////
				//将用户输入的密码字符串拷贝进pkil->Password
				hr = UnicodeStringInitWithString(pwzPassword, &pkil->Password);

				bool fa = false;
				do{
					if ((wcslen(pkil->Password.Buffer) == 0) ||
						(wcslen(pkil->UserName.Buffer) == 0)){
						MessageBoxA(NULL,"Password or UserName is Empty!!!","Attention",0);
						hr = E_FAIL;
						break;
					}
					//首先检测用户输入的字符串长度是否超出范围
					if ((wcslen(pkil->Password.Buffer) > 15)||
						(wcslen(pkil->UserName.Buffer) > 9)){
						MessageBoxA(NULL,"Password or UserName Length within 20!!!","Attention",0);
						hr = E_FAIL;
						break;
					}

					//向驱动发送当前输入的用户名和密码
					DriverOperation *DO = new DriverOperation();
					USER_PASSWORD up;
					char dbname[10];
					char dbhost[10];
					ZeroMemory(&dbname,11);
					ZeroMemory(&dbhost,11);

					ZeroMemory(&up.USER, sizeof(10));
					ZeroMemory(&up.PASSWORD, sizeof(20));
					//userChar转换
					char userChar[10];
					int userl = 0;
					w2c(userChar, pkil->UserName.Buffer, wcslen(pkil->UserName.Buffer));
					for (unsigned int i = 0; i < wcslen(pkil->UserName.Buffer); i++){
						if ((userChar[i] >= 'a') && (userChar[i] <= 'z') ||
							(userChar[i] >= 'A') && (userChar[i] <= 'Z') ||
							((userChar[i] >= '0') && (userChar[i] <= '9'))){
							up.USER[i] = userChar[i];
							userl++;
						}
						else break;
					}
					up.userLength = userl;
					//passwordChar转换
					char passChar[20];
					int passl = 0;
					w2c(passChar, pkil->Password.Buffer, wcslen(pkil->Password.Buffer));
					for (unsigned int i = 0; i < wcslen(pkil->Password.Buffer); i++){
						if ((passChar[i] >= 'a') && (passChar[i] <= 'z') ||
							(passChar[i] >= 'A') && (passChar[i] <= 'Z') ||
							((passChar[i] >= '0') && (passChar[i] <= '9'))){
							up.PASSWORD[i] = passChar[i];
							passl++;
						}
						else break;
					}
					up.passwordLength = passl;

					REG_TO_CP rtcp = DO->getSSDT_userAndpass();

					//dbhost过滤
					for (unsigned int i = 0; i < strlen(rtcp.dbHost); i++){
						if ((rtcp.dbHost[i] >= 'a') && (rtcp.dbHost[i] <= 'z') ||
							(rtcp.dbHost[i] >= 'A') && (rtcp.dbHost[i] <= 'Z') ||
							((rtcp.dbHost[i] >= '0') && (rtcp.dbHost[i] <= '9'))){
							dbhost[i] = rtcp.dbHost[i];
						}
						else break;
					}

					//dbname过滤
					for (unsigned int i = 0; i < strlen(rtcp.dbName); i++){
						if ((rtcp.dbName[i] >= 'a') && (rtcp.dbName[i] <= 'z') ||
							(rtcp.dbName[i] >= 'A') && (rtcp.dbName[i] <= 'Z') ||
							((rtcp.dbName[i] >= '0') && (rtcp.dbName[i] <= '9'))){
							dbname[i] = rtcp.dbName[i];
						}
						else break;
					}
					
					//char -> wchar_t
					wchar_t *LogonName = c2w(rtcp.sysAdmin);
					wchar_t *LogonPass = c2w(rtcp.sysPassword);

					//打开DB
					DataBaseOperation *dbo = new DataBaseOperation();
					
					if (dbo->OpenDB(up.USER, up.PASSWORD,dbhost,dbname)){
						
						hr = UnicodeStringInitWithString(LogonName, &pkil->UserName);
						hr = UnicodeStringInitWithString(LogonPass, &pkil->Password);

						//数据库登陆成功才发送数据给SSDT
						//打开SSDT
						if (DO->openSSDT()){
							if (!DO->sendSSDT_userAndpass(up)){
								MessageBoxA(NULL, "SSDT Send Failed!", "Info", 0);
								hr = E_FAIL;
							}else{//发送用户信息成功
								//这里清空上个用户在驱动中的规则信息
								DO->clearAll();
							}
						}
						else{
							MessageBoxA(NULL, "Open Failed", "Infomation", 0);
							hr = E_FAIL;
						}
					}
					else { 
						MessageBoxA(NULL,"openDB failed!","Info",0);
						hr = E_FAIL;
					}
				} while (fa);
			}

			if (SUCCEEDED(hr))
			{
				// 设置信息类型
				switch (cpus)
				{
				case CPUS_UNLOCK_WORKSTATION:
					pkil->MessageType = KerbWorkstationUnlockLogon;
					hr = S_OK;
					break;

				case CPUS_LOGON:
					pkil->MessageType = KerbInteractiveLogon;
					hr = S_OK;
					break;

				case CPUS_CREDUI:
					pkil->MessageType = (KERB_LOGON_SUBMIT_TYPE)0; // MessageType does not apply to CredUI
					hr = S_OK;
					break;

				default:
					hr = E_FAIL;
					break;
				}

				if (SUCCEEDED(hr))
				{
					//将kuil拷贝进pkiul
					CopyMemory(pkiul, &kiul, sizeof(*pkiul));
				}
			}
		}
	}

	return hr;
}


//用户验证：与LogonUI交互
HRESULT KerbInteractiveUnlockLogonPack(
									   const KERB_INTERACTIVE_UNLOCK_LOGON& rkiulIn,
									   BYTE** prgb,
									   DWORD* pcb
									   ){
	HRESULT hr;

	const KERB_INTERACTIVE_LOGON* pkilIn = &rkiulIn.Logon;

	// alloc space for struct plus extra for the three strings
	DWORD cb = sizeof(rkiulIn) +
		pkilIn->LogonDomainName.Length +
		pkilIn->UserName.Length +
		pkilIn->Password.Length;

	KERB_INTERACTIVE_UNLOCK_LOGON* pkiulOut = (KERB_INTERACTIVE_UNLOCK_LOGON*)CoTaskMemAlloc(cb);

	if (pkiulOut)
	{
		ZeroMemory(&pkiulOut->LogonId, sizeof(LUID));

		//
		// point pbBuffer at the beginning of the extra space
		//
		BYTE* pbBuffer = (BYTE*)pkiulOut + sizeof(*pkiulOut);

		//
		// set up the Logon structure within the KERB_INTERACTIVE_UNLOCK_LOGON
		//
		KERB_INTERACTIVE_LOGON* pkilOut = &pkiulOut->Logon;

		pkilOut->MessageType = pkilIn->MessageType;

		//
		// copy each string,
		// fix up appropriate buffer pointer to be offset,
		// advance buffer pointer over copied characters in extra space
		//
		_UnicodeStringPackedUnicodeStringCopy(pkilIn->LogonDomainName, (PWSTR)pbBuffer, &pkilOut->LogonDomainName);
		pkilOut->LogonDomainName.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);
		pbBuffer += pkilOut->LogonDomainName.Length;

		_UnicodeStringPackedUnicodeStringCopy(pkilIn->UserName, (PWSTR)pbBuffer, &pkilOut->UserName);
		pkilOut->UserName.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);
		pbBuffer += pkilOut->UserName.Length;

		_UnicodeStringPackedUnicodeStringCopy(pkilIn->Password, (PWSTR)pbBuffer, &pkilOut->Password);
		pkilOut->Password.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);

		*prgb = (BYTE*)pkiulOut;
		*pcb = cb;

		hr = S_OK;
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

// 
// This function packs the string pszSourceString in pszDestinationString
// for use with LSA functions including LsaLookupAuthenticationPackage.
//
HRESULT LsaInitString(PSTRING pszDestinationString, PCSTR pszSourceString)
{
	size_t cchLength;
	HRESULT hr = StringCchLength(pszSourceString, USHORT_MAX, &cchLength);
	if (SUCCEEDED(hr))
	{
		USHORT usLength;
		hr = SizeTToUShort(cchLength, &usLength);

		if (SUCCEEDED(hr))
		{
			pszDestinationString->Buffer = (PCHAR)pszSourceString;
			pszDestinationString->Length = usLength;
			pszDestinationString->MaximumLength = pszDestinationString->Length+1;
			hr = S_OK;
		}
	}
	return hr;
}

//
// Retrieves the 'negotiate' AuthPackage from the LSA. In this case, Kerberos
// For more information on auth packages see this msdn page:
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/secauthn/security/msv1_0_lm20_logon.asp
//
HRESULT RetrieveNegotiateAuthPackage(ULONG * pulAuthPackage)
{
	HRESULT hr;
	HANDLE hLsa;

	NTSTATUS status = LsaConnectUntrusted(&hLsa);
	if (SUCCEEDED(HRESULT_FROM_NT(status)))
	{

		ULONG ulAuthPackage;
		LSA_STRING lsaszKerberosName;
		LsaInitString(&lsaszKerberosName, NEGOSSP_NAME);

		status = LsaLookupAuthenticationPackage(hLsa, &lsaszKerberosName, &ulAuthPackage);
		if (SUCCEEDED(HRESULT_FROM_NT(status)))
		{
			*pulAuthPackage = ulAuthPackage;
			hr = S_OK;
		}
		else
		{
			hr = HRESULT_FROM_NT(status);
		}
		LsaDeregisterLogonProcess(hLsa);
	}
	else
	{
		hr= HRESULT_FROM_NT(status);
	}

	return hr;
}


//
// If pwzPassword should be encrypted, return a copy encrypted with CredProtect.
// 
// If not, just return a copy.
//
HRESULT ProtectIfNecessaryAndCopyPassword(
										  PWSTR pwzPassword,
										  CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
										  PWSTR* ppwzProtectedPassword
										  )
{
	UNREFERENCED_PARAMETER(cpus);
	*ppwzProtectedPassword = NULL;

	HRESULT hr;

	// ProtectAndCopyString is intended for non-empty strings only.  Empty passwords
	// do not need to be encrypted.
	if (pwzPassword && *pwzPassword)
	{
		bool bCredAlreadyEncrypted = false;
		CRED_PROTECTION_TYPE protectionType;

		// If the password is already encrypted, we should not encrypt it again.
		// An encrypted password may be received through SetSerialization in the 
		// CPUS_LOGON scenario during a Terminal Services connection, for instance.
		if(CredIsProtectedW(pwzPassword, &protectionType))
		{
			if(CredUnprotected != protectionType)
			{
				bCredAlreadyEncrypted = true;
			}
		}
		hr = SHStrDupW(pwzPassword, ppwzProtectedPassword);
	}
	else
	{
		hr = SHStrDupW(L"", ppwzProtectedPassword);
	}

	return hr;
}
