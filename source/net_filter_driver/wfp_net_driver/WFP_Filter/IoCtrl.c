#include "FilterLayers.h"
#include "IoCtrl.h"
#include <ndis.h>
#include <wdm.h>
#define INITGUID

extern KSPIN_LOCK DataListLock;
extern LIST_ENTRY DataList;

//---------------------------------------

//同步事件对象  
extern PRKEVENT g_pEventObject;
//句柄信息  
extern OBJECT_HANDLE_INFORMATION g_ObjectHandleInfo;
//是否开始监视
extern BOOLEAN StartMonitor;

//---------------------------------------

//Device Io Control
NTSTATUS TransferControl(PDEVICE_OBJECT pDO, PIRP pIrp,int *Info){
	KdPrint(("Transfer Data Control\n"));
	NTSTATUS nStatus = STATUS_SUCCESS;

	PIO_STACK_LOCATION	IrpStack = NULL;
	PTRANSFER_R3 pSystemBuffer = NULL;
	PRULE_FROM_R3 prfr = NULL;
	ULONG uInLen = 0;
	
	IrpStack = IoGetCurrentIrpStackLocation(pIrp);

	if (IrpStack != NULL)
	{
		uInLen = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
		if (IrpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL)
		{
			// 开始处理DeivceIoControl的情况
			switch (IrpStack->Parameters.DeviceIoControl.IoControlCode)
			{
			case IOCTL_WFP_MONITOR://用户层获得被监控的信息
				pSystemBuffer = (PTRANSFER_R3)pIrp->AssociatedIrp.SystemBuffer;
				KdPrint(("-----------Monitor-----------\n"));
				if (IsListEmpty(&DataList))
					KdPrint(("DataList is Empty!\n"));
				else{
					MonitorData(pSystemBuffer);
				}
					
				KdPrint(("-------END----Monitor--------\n"));
				*Info = sizeof(TRANSFER_R3);
				break;
			case IOCTL_WFP_FORBIDDEN://驱动层增加规则库
				prfr = (PRULE_FROM_R3)pIrp->AssociatedIrp.SystemBuffer;
				KdPrint(("-----------Forbidden-----------\n"));
				KdPrint(("imageName From App :%s\n", prfr->ImageName));
				nStatus = AddRuleFromR3(prfr);
				KdPrint(("-------END----Forbidden--------\n"));
				*Info = 0;
				break;
			case IOCTL_WFP_REMOVE_RULE://驱动层移除指定规则
				prfr = (PRULE_FROM_R3)pIrp->AssociatedIrp.SystemBuffer;
				KdPrint(("----------RemoveRule-----------\n"));
				if (STATUS_SUCCESS == RemoveRule(prfr))
					KdPrint(("RemoveListSuccess!\n"));
				else
					KdPrint(("RemoveListFailed!\n"));
				KdPrint(("-------END----RemoveRule-------\n"));
				*Info = 0;
				break;
			case IOCTL_WFP_START_EVENT://开始事件
				pSystemBuffer = (PTRANSFER_R3)pIrp->AssociatedIrp.SystemBuffer;
				KdPrint(("-------start EVENT-------\n"));
				//设置同步事件  
				if (pSystemBuffer == NULL || uInLen < sizeof(HANDLE)){
					KdPrint(("Set Event Error~!\n"));
					break;
				}
				//取得句柄对象  
				HANDLE hEvent = *(HANDLE*)pSystemBuffer;
				nStatus = ObReferenceObjectByHandle(hEvent, GENERIC_ALL, NULL, KernelMode, (PVOID*)&g_pEventObject, &g_ObjectHandleInfo);
				KdPrint(("g_pEventObject = 0x%X\n", g_pEventObject));
				//倘若没有开始监视,设置为开始监视
				if (!StartMonitor){
					StartMonitor = TRUE;
				}
				KdPrint(("-------end  start EVENT-------\n"));
				break;
			case IOCTL_WFP_STOP_EVENT://停止事件
				KdPrint(("-------stop EVENT-------\n"));
				if (StartMonitor){
					StartMonitor = FALSE;
				}
				//释放对象引用  
				if (g_pEventObject != NULL){
					ObDereferenceObject(g_pEventObject);
					g_pEventObject = NULL;
				}
				KdPrint(("-------end stop EVENT-------\n"));
				break;
			case IOCTL_WFP_CLEAR_BLACK_LIST:
				WfpRuleListUnInit();
				break;
			default:
				nStatus = STATUS_UNSUCCESSFUL;
			}
		}
	}
	return nStatus;
}

//监视命令，也就是应用层读取内核层当前信息
VOID MonitorData(PTRANSFER_R3 pSystemBuffer){
	KdPrint(("Monitor Data\n"));

	KLOCK_QUEUE_HANDLE handle;
	KeAcquireInStackQueuedSpinLock(&DataListLock, &handle);

	//KdPrint(("DataList is Not Empty!\n"));
	LIST_ENTRY* entry = RemoveHeadList(&DataList);
	PBIND_DATA_LIST data = CONTAINING_RECORD(entry, BIND_DATA_LIST, listEntry);
	PEPROCESS epro = NULL;
	UCHAR *pImageName = NULL;

	//获得PEPROCESS对象
	epro = LookupProcess((HANDLE)data->uProcessID);
	if (epro)
		pImageName = PsGetProcessImageFileName(epro);

	memcpy(data->imageName, pImageName, MAX_NAME);

	//KdPrint(("data->imageName:%s\n", data->imageName));

	//IsSend
	pSystemBuffer->bIsSend = data->bIsSend;
	//数据长度
	pSystemBuffer->uDataLength = data->uDataLength;
	//KdPrint(("pSystemBuffer->uDataLength:%d\n", pSystemBuffer->uDataLength));
	//本地端口
	pSystemBuffer->uLocalPort = data->uLocalPort;
	//远程端口
	pSystemBuffer->uRemotePort = data->uRemotePort;
	//进程ID
	pSystemBuffer->uProcessID = data->uProcessID;
	//本地IP
	pSystemBuffer->SourceIp = data->localAddressV4;
	//远程IP
	pSystemBuffer->RemoteIp = data->remoteAddressV4;

	//imageName拷贝
	for (int i = 0; i < 64; i++){
		pSystemBuffer->imageName[i] = data->imageName[i];
	}

	//processPath拷贝
	for (int j = 0; j < 1024; j++){
		pSystemBuffer->wProcessPath[j] = data->wProcessPath[j];
	}

	//KdPrint(("ProcessID: %d\n", pSystemBuffer->uProcessID));
	//KdPrint(("ProcessPath: %ws\n", pSystemBuffer->wProcessPath));
	//KdPrint(("ImageName: %s\n", pSystemBuffer->imageName));

	ExFreePoolWithTag(data, 'ttr');

	KeReleaseInStackQueuedSpinLock(&handle);
}

//根据进程ID返回进程EPROCESS，失败返回NULL
PEPROCESS LookupProcess(HANDLE Pid)
{
	PEPROCESS eprocess = NULL;
	if (NT_SUCCESS(PsLookupProcessByProcessId(Pid, &eprocess)))
		return eprocess;
	else
		return NULL;
}

