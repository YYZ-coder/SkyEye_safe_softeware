#include "FilterLayers.h"
#define INITGUID

PDEVICE_OBJECT g_pDeviceObj = NULL;
//Engine
HANDLE	g_hEngine = NULL;

//FlowContext List
LIST_ENTRY  FlowContextList;
KSPIN_LOCK  FlowContextListLock;
//BIND_DATA_LIST
LIST_ENTRY DataList;
KSPIN_LOCK DataListLock ;
//RULE_LIST
LIST_ENTRY RuleList ;
KSPIN_LOCK  RuleListLock ;

NTSTATUS IRPDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	KdPrint(("IRP Dispatch\n"));

	NTSTATUS nStatus = STATUS_SUCCESS;
	ULONG	ulInformation = 0;
	UNREFERENCED_PARAMETER(DeviceObject);
	do
	{
		if (Irp == NULL)
			break;
		nStatus = TransferControl(DeviceObject, Irp, &ulInformation);
		if (nStatus == STATUS_UNSUCCESSFUL){
			ulInformation = 0;
			KdPrint(("TransferControl Status Failed!\n"));
		}
		nStatus = STATUS_SUCCESS;

	} while (FALSE);

	if (Irp != NULL)
	{
		Irp->IoStatus.Information = ulInformation;
		Irp->IoStatus.Status = nStatus;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}
	return nStatus;
}

PDEVICE_OBJECT	CreateDevice(__in struct _DRIVER_OBJECT* DriverObject)
{
	KdPrint(("CreateDevice\n"));

	UNICODE_STRING	uDeviceName = { 0 };
	UNICODE_STRING	uSymbolName = { 0 };
	PDEVICE_OBJECT	pDeviceObj = NULL;
	NTSTATUS nStatsus = STATUS_UNSUCCESSFUL;
	RtlInitUnicodeString(&uDeviceName, WFP_DEVICE_NAME);
	RtlInitUnicodeString(&uSymbolName, WFP_SYM_LINK_NAME);
	//先删除符号链接，再进行创建
	IoDeleteSymbolicLink(&uSymbolName);

	nStatsus = IoCreateDevice(DriverObject,
		0, &uDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObj);

	if (pDeviceObj != NULL){
		pDeviceObj->Flags |= DO_BUFFERED_IO;
		pDeviceObj->AlignmentRequirement = FILE_WORD_ALIGNMENT;
		pDeviceObj->Flags &= ~DO_DEVICE_INITIALIZING;
	}

	IoCreateSymbolicLink(&uSymbolName, &uDeviceName);

	return pDeviceObj;
}

NTSTATUS DriverEntry(__in struct _DRIVER_OBJECT* DriverObject, __in PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	KdPrint(("DriverEntry\n"));

	NTSTATUS nStatus = STATUS_UNSUCCESSFUL;
	do
	{
		if (DriverObject == NULL)
			break;

		//FlowContextList
		InitializeListHead(&FlowContextList);
		KeInitializeSpinLock(&FlowContextListLock);
		//DataList
		InitializeListHead(&DataList);
		KeInitializeSpinLock(&DataListLock);
		//RuleList		
		InitializeListHead(&RuleList);
		KeInitializeSpinLock(&RuleListLock);

		DriverObject->MajorFunction[IRP_MJ_CREATE] = IRPDispatch;
		DriverObject->MajorFunction[IRP_MJ_CLOSE] = IRPDispatch;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IRPDispatch;

		if (STATUS_SUCCESS != InitWfp())
		{
			break;
		}
		DriverObject->DriverUnload = DriverUnload;

		g_pDeviceObj = CreateDevice(DriverObject);
		if (g_pDeviceObj == NULL)
		{
			break;
		}

		
		nStatus = STATUS_SUCCESS;
	} while (FALSE);

	if (nStatus != STATUS_SUCCESS)
	{
		UninitWfp();
		DeleteDevice();
	}
	return nStatus;
}

VOID DeleteDevice()
{
	KdPrint(("DeleteDevice\n"));
	UNICODE_STRING uSymbolName = { 0 };
	RtlInitUnicodeString(&uSymbolName, WFP_SYM_LINK_NAME);
	IoDeleteSymbolicLink(&uSymbolName);
	if (g_pDeviceObj != NULL)
	{
		IoDeleteDevice(g_pDeviceObj);
	}
	g_pDeviceObj = NULL;
}

VOID UninitWfp()
{
	KdPrint(("UninitWfp\n"));

	WfpEstablishUnInit();

	WfpDataUnInit();

	WfpRuleListUnInit();
	
	CloseEngine();
}

void CloseEngine()
{
	KdPrint(("CloseEngine\n"));

	if (g_hEngine != NULL)
	{
		FwpmEngineClose(g_hEngine);
	}
	g_hEngine = NULL;
}

void  DriverUnload(__in struct _DRIVER_OBJECT  *DriverObject)
{
	KdPrint(("DriverUnload\n"));
	UninitWfp();
	DeleteDevice();
	return;
}

void OpenEngine()
{
	KdPrint(("OpenEngine\n"));

	FWPM_SESSION0 Session = { 0 };
	FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, &Session, &g_hEngine);
}

NTSTATUS InitWfp()
{
	//BreakPoint------------------
	KdPrint(("InitWfp\n"));

	NTSTATUS nStatus = STATUS_UNSUCCESSFUL;
	do
	{
		//打开引擎
		OpenEngine();
		if (g_hEngine == NULL)
		{
			break;
		}

		if (STATUS_UNSUCCESSFUL == WfpEstablishInit()){
			break;
		}

		if(STATUS_UNSUCCESSFUL == WfpDataInit()){
			break;
		}
		WfpRuleListInit();

		nStatus = STATUS_SUCCESS;
	} while (FALSE);
	return nStatus;
}

