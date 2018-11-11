#include "Regedit_Operation.h"
#include <tchar.h>

#define  MY_REG_SOFTWARE_KEY_NAME		L"\\Registry\\Machine\\Software\\Owl"
#define  MY_REG_SOFTWARE_KEY_DB			L"\\Registry\\Machine\\Software\\Owl\\db"
#define	 MY_REG_SOFTWARE_KEY_SYS		L"\\Registry\\Machine\\Software\\Owl\\sys"

extern PCP_TO_R3 pc2r;

extern PREG_TO_CP pRTC;

extern BOOLEAN SkyEyeGetReg;

//创建键
NTSTATUS CreateKey(){
	NTSTATUS Status = STATUS_SUCCESS;
	//----------------主键---------------
	UNICODE_STRING RegUnicodeString;
	HANDLE hRegister;
	ULONG  ulResult;
	//初始化UNICODE_STRING字符串
	RtlInitUnicodeString(&RegUnicodeString,MY_REG_SOFTWARE_KEY_NAME);
	OBJECT_ATTRIBUTES objectAttributes;
	//初始化objectAttributes
	InitializeObjectAttributes(&objectAttributes, &RegUnicodeString, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	//创建或带开注册表项目
	NTSTATUS ntStatus = ZwCreateKey(&hRegister,KEY_ALL_ACCESS,&objectAttributes,0,NULL,REG_OPTION_NON_VOLATILE,&ulResult);
	if (NT_SUCCESS(ntStatus)){
		//判断是被新创建，还是已经被创建
		if (ulResult == REG_CREATED_NEW_KEY){
			KdPrint(("The register item is created\n"));
		}else if (ulResult == REG_OPENED_EXISTING_KEY){
			KdPrint(("The register item has been created,and now is opened\n"));
		}
	}

	//------------------子项-1----------------
	UNICODE_STRING subRegUnicodeString;
	HANDLE hSubRegister;
	//初始化UNICODE_STRING字符串
	RtlInitUnicodeString(&subRegUnicodeString,L"db");
	OBJECT_ATTRIBUTES subObjectAttributes;
	//初始化subObjectAttributes
	InitializeObjectAttributes(&subObjectAttributes,&subRegUnicodeString,OBJ_CASE_INSENSITIVE, hRegister,NULL);
	//创建或打开注册表项目
	ntStatus = ZwCreateKey(&hSubRegister,KEY_ALL_ACCESS,&subObjectAttributes,0,NULL,REG_OPTION_NON_VOLATILE,&ulResult);
	if (NT_SUCCESS(ntStatus)){
		//判断是被新创建，还是已经被创建
		if (ulResult == REG_CREATED_NEW_KEY){
			KdPrint(("The sub register db is created\n"));
		}else if (ulResult == REG_OPENED_EXISTING_KEY){
			KdPrint(("The sub register db has been created,and now is opened\n"));
		}
	}

	//------------------子项-2----------------
	UNICODE_STRING subRegisterString;
	HANDLE hSubRegister0;
	//初始化String
	RtlInitUnicodeString(&subRegisterString,L"sys");
	OBJECT_ATTRIBUTES subObjectAttribute0;
	//初始化subObjectAttribute
	InitializeObjectAttributes(&subObjectAttribute0,&subRegisterString,OBJ_CASE_INSENSITIVE,hRegister,NULL);
	//创建或大开注册表项目
	ntStatus = ZwCreateKey(&hSubRegister0,KEY_ALL_ACCESS,&subObjectAttribute0,0,NULL,REG_OPTION_NON_VOLATILE,&ulResult);
	if (NT_SUCCESS(ntStatus)){
		//判断是被新创建，还是已经被创建
		if (ulResult == REG_CREATED_NEW_KEY){
			KdPrint(("The sub register sys is created\n"));
		}
		else if (ulResult == REG_OPENED_EXISTING_KEY){
			KdPrint(("The sub register sys has been created,and now is opened\n"));
		}
	}

	//关闭注册表句柄
	ZwClose(hRegister);
	ZwClose(hSubRegister);
	ZwClose(hSubRegister0);
	return Status;
}

//设置注册表项DB键值
NTSTATUS SetDBKey(WCHAR *dbname,WCHAR *dbhost){
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	UNICODE_STRING ValueName;
	UNICODE_STRING RegUnicodeString;
	HANDLE hRegister;

	//初始化UNICODE_STRING字符串
	RtlInitUnicodeString(&RegUnicodeString, MY_REG_SOFTWARE_KEY_DB);
	OBJECT_ATTRIBUTES objectAttributes;
	//初始化objectAttributes
	InitializeObjectAttributes(&objectAttributes,&RegUnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);

	do{
		//打开注册表
		NTSTATUS ntStatus = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);
		if (NT_SUCCESS(ntStatus)){
			KdPrint(("Open register db successfully\n"));
		}
		else{ 
			KdPrint(("Open register db failed\n")); 
			break;
		}

		//REG_SZ
		//-----------------------------DBNAME-------------------------------------
		RtlInitUnicodeString(&ValueName, L"dbname");
		//WCHAR* dbname = L"mysql";
		Status = ZwSetValueKey(hRegister, &ValueName, 0, REG_SZ, dbname, wcslen(dbname) * 2 + 2);
		//------------------------------------------------------------------------

		//-----------------------------DBHOST-------------------------------------
		RtlInitUnicodeString(&ValueName,L"dbhost");
		//WCHAR* dbhost = L"localhost";
		Status = ZwSetValueKey(hRegister, &ValueName, 0, REG_SZ, dbhost, wcslen(dbhost) * 2 + 2);
		//------------------------------------------------------------------------

		//关闭注册表句柄
		ZwClose(hRegister);
	} while (FALSE);
	
	return Status;
}

//设置注册表项SYS键值
NTSTATUS SetSYSKey(WCHAR *sysAdmin, WCHAR *sysPassword){
	NTSTATUS Status = STATUS_SUCCESS;

	UNICODE_STRING ValueName;
	UNICODE_STRING RegUnicodeString;
	HANDLE hRegister;

	//初始化UNICODE_STRING字符串
	RtlInitUnicodeString(&RegUnicodeString, MY_REG_SOFTWARE_KEY_SYS);
	OBJECT_ATTRIBUTES objectAttributes;
	//初始化objectAttributes
	InitializeObjectAttributes(&objectAttributes, &RegUnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);

	do{
		//打开注册表
		NTSTATUS ntStatus = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);
		if (NT_SUCCESS(ntStatus)){
			KdPrint(("Open register sys successfully\n"));
		}
		else{
			KdPrint(("Open register sys failed\n"));
			break;
		}

		//REG_SZ
		//-----------------------------SYSADMIN-------------------------------------
		RtlInitUnicodeString(&ValueName, L"sysAdmin");
		//WCHAR* sysAdmin = L"root";
		Status = ZwSetValueKey(hRegister, &ValueName, 0, REG_SZ, sysAdmin, wcslen(sysAdmin) * 2 + 2);
		//------------------------------------------------------------------------

		//-----------------------------SYSPASSWORD-------------------------------------
		RtlInitUnicodeString(&ValueName, L"sysPassword");
		//WCHAR* sysPassword = L"123";
		Status = ZwSetValueKey(hRegister, &ValueName, 0, REG_SZ, sysPassword, wcslen(sysPassword) * 2 + 2);
		//------------------------------------------------------------------------

		//关闭注册表句柄
		ZwClose(hRegister);
	} while (FALSE);

	return Status;
}

//查询DB键值
VOID QueryDBKey(){

	UNICODE_STRING RegUnicodeString;
	HANDLE hRegister;
	NTSTATUS ntStatus;
	UNICODE_STRING ValueName;		//键值名
	ULONG ulSize;					//返回的值长度
	PKEY_VALUE_PARTIAL_INFORMATION pvpi;

	//初始化UNICODE_STRING字符串
	RtlInitUnicodeString(&RegUnicodeString,MY_REG_SOFTWARE_KEY_DB);

	OBJECT_ATTRIBUTES objectAttributes;
	//初始化objectAttributes
	InitializeObjectAttributes(&objectAttributes,&RegUnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);
	do{
		//打开注册表
		ntStatus = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);
		if (NT_SUCCESS(ntStatus)){
			KdPrint(("Open register successfully\n"));
		}
		else break;

		//REG_SZ
		//--------------------------------------------DBHOST-------------------------------------------------
		//初始化ValueName
		RtlInitUnicodeString(&ValueName, L"dbhost");
		//读取REG_SZ子键
		ntStatus = ZwQueryValueKey(hRegister,&ValueName,KeyValuePartialInformation,NULL,0,&ulSize);
		if (ntStatus == STATUS_OBJECT_NAME_NOT_FOUND || ulSize == 0){
			ZwClose(hRegister);
			KdPrint(("The item is not exist\n"));
			return ;
		}
		pvpi =(PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
		ntStatus = ZwQueryValueKey(hRegister,&ValueName,KeyValuePartialInformation,pvpi,ulSize,&ulSize);
		if (!NT_SUCCESS(ntStatus)){
			ZwClose(hRegister);
			KdPrint(("Read regsiter error\n"));
			return ;
		}
		//判断是否为REG_SZ类型
		if (pvpi->Type == REG_SZ){
			KdPrint(("The value:%S\n", pvpi->Data));
			int i = 0;
			//将读出来的值赋给rgr结构
			KdPrint(("dbhost length:%d\n",pvpi->DataLength));
			//-------------------------------------------------------------------
			ANSI_STRING asci;
			UNICODE_STRING unicode;
			unicode.Length = unicode.MaximumLength = (USHORT)pvpi->DataLength;
			unicode.Buffer = (PWSTR)ExAllocatePool(PagedPool, unicode.Length);
			//将查询到的数据pvpi拷贝给新建的UNICODE_STRING中
			RtlCopyMemory(unicode.Buffer, pvpi->Data, unicode.Length);

			KdPrint(("unicode buffer:%ws\n", unicode.Buffer));
			//将unicode转成Asci,并给Asci分配内存
			RtlUnicodeStringToAnsiString(&asci, &unicode, TRUE);

			KdPrint(("asci buffer:%s\n", asci.Buffer));

			if (SkyEyeGetReg){
				for (; i < strlen(asci.Buffer); i++){
					KdPrint(("asci.Buffer: %c\n", asci.Buffer[i]));
					pc2r->HOST[i] = asci.Buffer[i];
				}
			}
			else{
				for (; i < strlen(asci.Buffer); i++){
					pRTC->dbHost[i] = asci.Buffer[i];
				}
			}
			
			//-------------------------------------------------------------------
		}
		ExFreePool(pvpi);
		//---------------------------------------------------------------------------------------------------

		//--------------------------------------------DBNAME-------------------------------------------------
		//初始化ValueName
		RtlInitUnicodeString(&ValueName, L"dbname");
		//判断子键存在
		ntStatus = ZwQueryValueKey(hRegister, &ValueName, KeyValuePartialInformation, NULL, 0, &ulSize);
		if (ntStatus == STATUS_OBJECT_NAME_NOT_FOUND || ulSize == 0){
			ZwClose(hRegister);
			KdPrint(("The item is not exist\n"));
			return ;
		}
		pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
		ntStatus = ZwQueryValueKey(hRegister, &ValueName, KeyValuePartialInformation, pvpi, ulSize, &ulSize);
		if (!NT_SUCCESS(ntStatus)){
			ZwClose(hRegister);
			KdPrint(("Read regsiter error\n"));
			return ;
		}
		//判断是否为REG_SZ类型
		if (pvpi->Type == REG_SZ){
			KdPrint(("The value:%S\n", pvpi->Data));
			int i = 0;
			KdPrint(("dbname length:%d\n", pvpi->DataLength));

			//-------------------------------------------------------------------
			//PWCHAR namedata = (PWCHAR)pvpi->Data;
			ANSI_STRING asci;
			UNICODE_STRING unicode;
			unicode.Length = unicode.MaximumLength = (USHORT)pvpi->DataLength;
			unicode.Buffer = (PWSTR)ExAllocatePool(PagedPool,unicode.Length);

			RtlCopyMemory(unicode.Buffer,pvpi->Data,unicode.Length);

			KdPrint(("unicode buffer:%ws\n",unicode.Buffer));

			RtlUnicodeStringToAnsiString(&asci,&unicode,TRUE);

			KdPrint(("asci buffer:%s\n", asci.Buffer));
			if (SkyEyeGetReg){
				for (; i < strlen(asci.Buffer); i++){
					KdPrint(("asci.Buffer: %c\n", asci.Buffer[i]));
					pc2r->DBNAME[i] = asci.Buffer[i];
				}
			}
			else{
				for (; i < strlen(asci.Buffer); i++){
					pRTC->dbName[i] = asci.Buffer[i];
				}
			}
			
			//-------------------------------------------------------------------
		}
		ExFreePool(pvpi);
		//---------------------------------------------------------------------------------------------------

		ZwClose(hRegister);
	} while (FALSE);
}

//查询SYS键值
VOID QuerySYSKey(){

	UNICODE_STRING RegUnicodeString;
	HANDLE hRegister;
	NTSTATUS ntStatus;
	UNICODE_STRING ValueName;		//键值名
	ULONG ulSize;					//返回的值长度
	PKEY_VALUE_PARTIAL_INFORMATION pvpi;

	//初始化UNICODE_STRING字符串
	RtlInitUnicodeString(&RegUnicodeString, MY_REG_SOFTWARE_KEY_SYS);

	OBJECT_ATTRIBUTES objectAttributes;
	//初始化objectAttributes
	InitializeObjectAttributes(&objectAttributes, &RegUnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);
	do{
		//打开注册表
		ntStatus = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);
		if (NT_SUCCESS(ntStatus)){
			KdPrint(("Open register successfully\n"));
		}

		//REG_SZ
		//--------------------------------------------SYSAdmin-------------------------------------------------
		//初始化ValueName
		RtlInitUnicodeString(&ValueName, L"sysAdmin");
		//读取REG_SZ子键
		ntStatus = ZwQueryValueKey(hRegister, &ValueName, KeyValuePartialInformation, NULL, 0, &ulSize);
		if (ntStatus == STATUS_OBJECT_NAME_NOT_FOUND || ulSize == 0){
			ZwClose(hRegister);
			KdPrint(("The item is not exist\n"));
			return;
		}
		pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
		ntStatus = ZwQueryValueKey(hRegister, &ValueName, KeyValuePartialInformation, pvpi, ulSize, &ulSize);
		if (!NT_SUCCESS(ntStatus)){
			ZwClose(hRegister);
			KdPrint(("Read regsiter error\n"));
			return;
		}
		//判断是否为REG_SZ类型
		if (pvpi->Type == REG_SZ){
			KdPrint(("The value:%S\n", pvpi->Data));
			int i = 0;
			//-------------------------------------------------------------------
			ANSI_STRING asci;
			UNICODE_STRING unicode;
			unicode.Length = unicode.MaximumLength = (USHORT)pvpi->DataLength;
			unicode.Buffer = (PWSTR)ExAllocatePool(PagedPool, unicode.Length);
			//将查询到的数据pvpi拷贝给新建的UNICODE_STRING中
			RtlCopyMemory(unicode.Buffer, pvpi->Data, unicode.Length);

			KdPrint(("unicode buffer:%ws\n", unicode.Buffer));
			//将unicode转成Asci,并给Asci分配内存
			RtlUnicodeStringToAnsiString(&asci, &unicode, TRUE);

			KdPrint(("asci buffer:%s\n", asci.Buffer));

			for (; i < strlen(asci.Buffer); i++){
				pRTC->sysAdmin[i] = asci.Buffer[i];
			}
			//-------------------------------------------------------------------
		}
		ExFreePool(pvpi);
		//---------------------------------------------------------------------------------------------------

		//--------------------------------------------SysPassword-------------------------------------------------
		//初始化ValueName
		RtlInitUnicodeString(&ValueName, L"sysPassword");
		//判断子键存在
		ntStatus = ZwQueryValueKey(hRegister, &ValueName, KeyValuePartialInformation, NULL, 0, &ulSize);
		if (ntStatus == STATUS_OBJECT_NAME_NOT_FOUND || ulSize == 0){
			ZwClose(hRegister);
			KdPrint(("The item is not exist\n"));
			return;
		}
		pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
		ntStatus = ZwQueryValueKey(hRegister, &ValueName, KeyValuePartialInformation, pvpi, ulSize, &ulSize);
		if (!NT_SUCCESS(ntStatus)){
			ZwClose(hRegister);
			KdPrint(("Read regsiter error\n"));
			return;
		}
		//判断是否为REG_SZ类型
		if (pvpi->Type == REG_SZ){
			KdPrint(("The value:%S\n", pvpi->Data));
			int i = 0;
			//-------------------------------------------------------------------
			//PWCHAR namedata = (PWCHAR)pvpi->Data;
			ANSI_STRING asci;
			UNICODE_STRING unicode;
			unicode.Length = unicode.MaximumLength = (USHORT)pvpi->DataLength;
			unicode.Buffer = (PWSTR)ExAllocatePool(PagedPool, unicode.Length);
			//将查询到的数据pvpi拷贝给新建的UNICODE_STRING中
			RtlCopyMemory(unicode.Buffer, pvpi->Data, unicode.Length);

			KdPrint(("unicode buffer:%ws\n", unicode.Buffer));
			//将unicode转成Asci,并给Asci分配内存
			RtlUnicodeStringToAnsiString(&asci, &unicode, TRUE);

			KdPrint(("asci buffer:%s\n", asci.Buffer));

			for (; i < strlen(asci.Buffer); i++){
				pRTC->sysPassword[i] = asci.Buffer[i];
			}
			//-------------------------------------------------------------------
		}
		ExFreePool(pvpi);
		//---------------------------------------------------------------------------------------------------

		ZwClose(hRegister);
	} while (FALSE);

	//return STATUS_SUCCESS;
}

//删除DB键
NTSTATUS DeleteDBKey(){

	UNICODE_STRING RegUnicodeString;
	HANDLE hRegister;
	NTSTATUS ntStatus;

	//初始化UNICODE_STRING字符串
	RtlInitUnicodeString(&RegUnicodeString,MY_REG_SOFTWARE_KEY_DB);

	OBJECT_ATTRIBUTES objectAttributes;
	//初始化objectAttributes
	InitializeObjectAttributes(&objectAttributes,&RegUnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);
	//打开注册表
	ntStatus = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);

	if (NT_SUCCESS(ntStatus))
	{
		KdPrint(("Open register DB successfully\n"));
	}
	//删除指定的子键
	ntStatus = ZwDeleteKey(hRegister);
	if (NT_SUCCESS(ntStatus))
	{
		KdPrint(("Delete the DB successfully\n"));
	}
	else if (ntStatus == STATUS_ACCESS_DENIED)
	{
		KdPrint(("STATUS_ACCESS_DENIED\n"));

	}
	else if (ntStatus == STATUS_INVALID_HANDLE)
	{
		KdPrint(("STATUS_INVALID_HANDLE\n"));
	}
	else
	{
		KdPrint(("Maybe the item has sub item to delete\n"));
	}

	ZwClose(hRegister);

	return STATUS_SUCCESS;
}

//删除SYS键
NTSTATUS DeleteSYSKey(){

	UNICODE_STRING RegUnicodeString;
	HANDLE hRegister;
	NTSTATUS ntStatus;

	//初始化UNICODE_STRING字符串
	RtlInitUnicodeString(&RegUnicodeString,MY_REG_SOFTWARE_KEY_SYS);

	OBJECT_ATTRIBUTES objectAttributes;
	//初始化objectAttributes
	InitializeObjectAttributes(&objectAttributes,&RegUnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);
	//打开注册表
	ntStatus = ZwOpenKey(&hRegister,KEY_ALL_ACCESS,&objectAttributes);
	if (NT_SUCCESS(ntStatus)){
		KdPrint(("Open register SYS successfully\n"));
	}
	//删除子项操作
	ntStatus = ZwDeleteKey(hRegister);
	if (NT_SUCCESS(ntStatus))
	{
		KdPrint(("Delete the SYS successfully\n"));
	}
	else if (ntStatus == STATUS_ACCESS_DENIED)
	{
		KdPrint(("STATUS_ACCESS_DENIED\n"));
	}
	else if (ntStatus == STATUS_INVALID_HANDLE)
	{
		KdPrint(("STATUS_INVALID_HANDLE\n"));
	}
	else
	{
		KdPrint(("Maybe the item has sub item to delete\n"));
	}

	ZwClose(hRegister);
	return STATUS_SUCCESS;
}

