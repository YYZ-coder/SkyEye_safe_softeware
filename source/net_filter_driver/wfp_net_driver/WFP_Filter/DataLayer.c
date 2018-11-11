#include "FilterLayers.h"
#include <ndis.h>

extern PDEVICE_OBJECT g_pDeviceObj;

//---------------------------------------

//开始监控的状态（初始状态设置为FALSE）
BOOLEAN StartMonitor = FALSE;

//同步事件对象  
PRKEVENT g_pEventObject = NULL;
//句柄信息  
OBJECT_HANDLE_INFORMATION g_ObjectHandleInfo;

//---------------------------------------

//ID
UINT32	g_uFwpsDataCallOutId = 0;
UINT32	g_uFwpmDataCallOutId = 0;
UINT64	g_uDataFilterId = 0;

//BIND_DATA_LIST
extern LIST_ENTRY DataList;
extern KSPIN_LOCK DataListLock;

//Engine
extern HANDLE	g_hEngine;

NTSTATUS WfpDataInit(){
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	do{
		if (STATUS_SUCCESS != WfpDataRegisterCallouts())
			break;

		if (STATUS_SUCCESS != WfpDataAddCallouts())
			break;

		if (STATUS_SUCCESS != WfpDataAddSubLayer())
			break;

		if (STATUS_SUCCESS != WfpDataAddFilters())
			break;

		Status = STATUS_SUCCESS;
	} while (FALSE);

	return Status;
}

VOID WfpDataUnInit(){
	WfpDataRemoveCallouts();
	WfpDataRemoveSubLayer();
	WfpDataRemoveFilters();
	WfpDataUnRegisterCallouts();

	KdPrint(("Data List UnInit\n"));
	PLIST_ENTRY entry;
	KLOCK_QUEUE_HANDLE handle;
	KeAcquireInStackQueuedSpinLock(&DataListLock,&handle);

	while(!IsListEmpty(&DataList)){
		entry = RemoveTailList(&DataList);
		PBIND_DATA_LIST data = CONTAINING_RECORD(entry, BIND_DATA_LIST, listEntry);
		ExFreePool(data);
	}

	KeReleaseInStackQueuedSpinLock(&handle);
}

//注册FWPS_CALLOUT
NTSTATUS WfpDataRegisterCallouts(){
	KdPrint(("WfpData Register Callout.\n"));

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	do{
		if (g_pDeviceObj == NULL)
			break;

		FWPS_CALLOUT sCallout;

		memset(&sCallout, 0, sizeof(FWPS_CALLOUT));

		sCallout.calloutKey = WFP_DATA_CALLOUT_V4_GUID;
		sCallout.flags = 0;
		sCallout.classifyFn = Data_ClassifyFn_V4;
		sCallout.notifyFn = Data_NotifyFn_V4;
		sCallout.flowDeleteFn = Data_FlowDeleteFn_V4;

		Status = FwpsCalloutRegister(g_pDeviceObj, &sCallout, &g_uFwpsDataCallOutId);

	} while (FALSE);

	return Status;
}

//添加FWPM_CALLOUT
NTSTATUS WfpDataAddCallouts(){
	KdPrint(("WfpData AddCallouts\n"));

	NTSTATUS status = STATUS_SUCCESS;

	FWPM_CALLOUT fwpmCallout = { 0 };
	fwpmCallout.flags = 0;
	do{
		if (g_hEngine == NULL)
		{
			break;
		}
		//displayData：为了给出FWPM_CALLOUT的名字和详细信息
		fwpmCallout.displayData.name = (wchar_t *)WFP_DATA_CALLOUT_DISPLAY_NAME;
		fwpmCallout.displayData.description = (wchar_t *)WFP_DATA_CALLOUT_DISPLAY_NAME;

		//uniquely identifies：必须与FWPS_CALLOUT的ID一样
		fwpmCallout.calloutKey = WFP_DATA_CALLOUT_V4_GUID;

		//applcableLayer：filtering layer at which the callout is applicable.
		fwpmCallout.applicableLayer = FWPM_LAYER_STREAM_V4;

		//向g_hEngine中加入
		status = FwpmCalloutAdd(g_hEngine, &fwpmCallout, NULL, &g_uFwpmDataCallOutId);

		if (!NT_SUCCESS(status) && (status != STATUS_FWP_ALREADY_EXISTS))
			break;
		status = STATUS_SUCCESS;
	} while (FALSE);
	return status;
}

//为Engine添加子层
NTSTATUS WfpDataAddSubLayer(){
	KdPrint(("WfpData AddSubLayer\n"));

	NTSTATUS nStatus = STATUS_UNSUCCESSFUL;
	FWPM_SUBLAYER SubLayer = { 0 };
	SubLayer.flags = 0;
	SubLayer.displayData.description = WFP_DATA_SUB_LAYER_DISPLAY_NAME;
	SubLayer.displayData.name = WFP_DATA_SUB_LAYER_DISPLAY_NAME;
	SubLayer.subLayerKey = WFP_DATA_SUBLAYER_GUID;
	SubLayer.weight = 65535;
	if (g_hEngine != NULL)
	{
		nStatus = FwpmSubLayerAdd(g_hEngine, &SubLayer, NULL);
	}
	return nStatus;
}

//为Engine添加过滤器
NTSTATUS WfpDataAddFilters(){
	KdPrint(("WfpData AddFilters\n"));

	NTSTATUS nStatus = STATUS_UNSUCCESSFUL;
	do
	{
		FWPM_FILTER0 Filter = { 0 };
		FWPM_FILTER_CONDITION FilterCondition[1] = { 0 };
		FWP_V4_ADDR_AND_MASK AddrAndMask = { 0 };
		if (g_hEngine == NULL)
		{
			break;
		}
		Filter.displayData.description = WFP_FILTER_DATA_DISPLAY_NAME;
		Filter.displayData.name = WFP_FILTER_DATA_DISPLAY_NAME;
		Filter.flags = 0;
		Filter.layerKey = FWPM_LAYER_STREAM_V4;// FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4;
		Filter.subLayerKey = WFP_DATA_SUBLAYER_GUID;
		Filter.weight.type = FWP_EMPTY;
		Filter.numFilterConditions = 1;
		Filter.filterCondition = FilterCondition;
		Filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
		Filter.action.calloutKey = WFP_DATA_CALLOUT_V4_GUID;

		FilterCondition[0].fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
		FilterCondition[0].matchType = FWP_MATCH_EQUAL;
		FilterCondition[0].conditionValue.type = FWP_V4_ADDR_MASK;
		FilterCondition[0].conditionValue.v4AddrMask = &AddrAndMask;

		nStatus = FwpmFilterAdd(g_hEngine, &Filter, NULL, &g_uDataFilterId);
		if (STATUS_SUCCESS != nStatus)
		{
			break;
		}
		nStatus = STATUS_SUCCESS;
	} while (FALSE);
	return nStatus;
}

//移除Stream layer 的CallOut (FWPM_CALLOUT)
VOID WfpDataRemoveCallouts()
{
	KdPrint(("WfpData RemoveCallouts\n"));

	if (g_hEngine != NULL)
	{
		FwpmCalloutDeleteById(g_hEngine, g_uFwpmDataCallOutId);
		g_uFwpmDataCallOutId = 0;
	}

}

//移除Stream layer 的SubLayer
VOID WfpDataRemoveSubLayer()
{
	KdPrint(("WfpData RemoveSubLayer\n"));

	if (g_hEngine != NULL)
	{
		FwpmSubLayerDeleteByKey(g_hEngine, &WFP_DATA_SUBLAYER_GUID);
	}
}

//移除Stream layer 的Filter
VOID WfpDataRemoveFilters()
{
	KdPrint(("WfpData RemoveFilters\n"));

	if (g_hEngine != NULL)
	{
		FwpmFilterDeleteById(g_hEngine, g_uDataFilterId);
	}
}

//UnRegister CallOut (FWPS_CALLOUT)
VOID WfpDataUnRegisterCallouts()
{
	KdPrint(("WfpData UnRegisterCallouts\n"));

	FwpsCalloutUnregisterById(g_uFwpsDataCallOutId);
	g_uFwpsDataCallOutId = 0;
}

//获得NBL的长度
ULONG GetNBLLength(PNET_BUFFER_LIST NBL){
	ULONG Length = 0;
	PNET_BUFFER	NB = NULL;

	NB = NET_BUFFER_LIST_FIRST_NB(NBL);

	for (;;)
	{
		if (NB == NULL) break;

		Length += NET_BUFFER_DATA_LENGTH(NB);
		//KdPrint(("NB Data Length: %d\n", Length));
		NB = NET_BUFFER_NEXT_NB(NB);
	}
	KdPrint(("Sum Length: %d \n", Length));
	//KdPrint(("Get NBLLength End.\n"));
	return Length;
}

//主要进行过滤，输出包头信息
VOID NTAPI Data_ClassifyFn_V4(
	IN const FWPS_INCOMING_VALUES  *inFixedValues,
	IN const FWPS_INCOMING_METADATA_VALUES  *inMetaValues,
	IN OUT VOID  *layerData,
	IN OPTIONAL const void  *classifyContext,
	IN const FWPS_FILTER1  *filter,
	IN UINT64  flowContext,
	OUT FWPS_CLASSIFY_OUT  *classifyOut
	){

	KdPrint(("--------------Wfp_Data_ClassifyFn_V4--------------\n"));

	NTSTATUS	Status = STATUS_SUCCESS;

	FLOW_DATA * flowData = NULL;
	WORD	wDirection = 0;
	WORD	wProtocol = 0;
	ULONG	Length = 0;
	PBIND_DATA_LIST bd = NULL;

	FWPS_STREAM_CALLOUT_IO_PACKET* StreamPacket = NULL;
	FWPS_STREAM_DATA* StreamBuffer = 0;
	FWPS_STREAM_DATA* tmpStream = NULL;
	PNET_BUFFER_LIST ppnbl = NULL;

	UINT64 uDataLeng = 0;

	if (!(classifyOut->rights & FWPS_RIGHT_ACTION_WRITE))
		return;

	bd = ExAllocatePoolWithTag(NonPagedPool,sizeof(BIND_DATA_LIST),'ttr');
	RtlZeroMemory(bd, sizeof(BIND_DATA_LIST));

	//获得Established layer 获取的所有包头以及进程信息
	flowData = *(FLOW_DATA**)(UINT64*)&flowContext;

	//KdPrint(("DataLayer Process ID: %d\n",flowData->processID));

	//KdPrint(("DataLayer Process Path: %ws\n\n",flowData->processPath));

	//默认"允许"(PERMIT)
	classifyOut->actionType = FWP_ACTION_PERMIT;

	do{
		//wDirection表示数据包的方向,取值为	//FWP_DIRECTION_INBOUND/FWP_DIRECTION_OUTBOUND
		wDirection = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_DIRECTION].value.int8;

		StreamPacket = (FWPS_STREAM_CALLOUT_IO_PACKET*)layerData;
		StreamBuffer = StreamPacket->streamData;
		uDataLeng = StreamPacket->streamData->dataLength;
	
		if (uDataLeng == 0)
			break;
		//KdPrint(("StreamPacket->streamData->dataLength:%d\n",uDataLeng));
		//数据长度
		bd->uDataLength = uDataLeng;
		//KdPrint(("bd->uDataLength:%d\n",bd->uDataLength));
		//本地端口
		bd->uLocalPort = flowData->localPort;
		//远程端口
		bd->uRemotePort = flowData->remotePort;
		//本地Ip
		bd->localAddressV4 = flowData->localAddressV4;
		//远程Ip
		bd->remoteAddressV4 = flowData->remoteAddressV4;
		//进程ID
		bd->uProcessID = flowData->processID;
		//进程路径
		wcsncpy(bd->wProcessPath, flowData->processPath, 1024);

		//ImageName Copy
		PEPROCESS epro = NULL;
		UCHAR *pImageName = NULL;
		epro = LookupProcess((HANDLE)bd->uProcessID);
		if (epro)
			pImageName = PsGetProcessImageFileName(epro);
		//ImageName的具体长度
		int sizepIN = strlen(pImageName);
		for (int i = 0; i < sizepIN; i++)
			bd->imageName[i] = pImageName[i];

		//OutPut IP
		UINT8 *lA = (UINT8 *)&bd->localAddressV4;
		UINT8 *rA = (UINT8 *)&bd->remoteAddressV4;

		//KdPrint((" DataLength :%d\n LocalPort : %d\n RemotePort : %d\n "
			//	"LocalAddress : %d,%d,%d,%d\n RemoteAddress : %d,%d,%d,%d\n "
			//	"ProcessID : %d\n ",
		//	bd->uDataLength,bd->uLocalPort,bd->uRemotePort,
		//	lA[3],lA[2],lA[1],lA[0],rA[3],rA[2],rA[1],rA[0],
//			bd->uProcessID));

		//OutPut ProcessPath
		//KdPrint(("ProcessPath : %ws\n", bd->wProcessPath));

		if (wDirection == FWP_DIRECTION_INBOUND){
			//KdPrint(("Stream Layer IN BOUND \n"));
			bd->bIsSend = FALSE;
		}
		else if (wDirection == FWP_DIRECTION_OUTBOUND){
			//KdPrint(("Stream Layer OUT BOUND \n"));
			bd->bIsSend = TRUE;
		}

		//表示含有禁止信息并进行阻断,匹配成功
		if (STATUS_SUCCESS == Data_Filtering(bd)){
			KdPrint(("----DataFiltering Forbidden Data----\n"));
			classifyOut->actionType = FWP_ACTION_BLOCK;
		}
		else{//表示不含有禁止的信息,匹配不成功
			//插入到BindDataList链表中
			//KdPrint(("----DataFiltering Allow Data----\n"));
			Status = InsertDataList(bd);
			if (!NT_SUCCESS(Status)){
				//KdPrint(("Stream InsertDataList Unsuccessful!\n"));
				break;
			}
			if (StartMonitor){
				//设置事件为有信号，通知应用层  
				KeSetEvent(g_pEventObject, 0, FALSE);
			}
		}
	} while (FALSE);
	
	//clear FWPS_RIGHT_ACTION_WRITE Tag
	if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT)
	{
		classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
	}
	KdPrint(("---------------------------------------"));
	return;
}

NTSTATUS NTAPI Data_NotifyFn_V4(
	IN  FWPS_CALLOUT_NOTIFY_TYPE        notifyType,
	IN  const GUID*             filterKey,
	IN  const FWPS_FILTER*     filter){
	return STATUS_SUCCESS;
}

VOID NTAPI Data_FlowDeleteFn_V4(
	IN UINT16  layerId,
	IN UINT32  calloutId,
	IN UINT64  flowContext
	){

}

//插入BIND_DATA
NTSTATUS InsertDataList(BIND_DATA_LIST *BD){
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	KLOCK_QUEUE_HANDLE LockHandle;

	KeAcquireInStackQueuedSpinLock(&DataListLock, &LockHandle);

	InsertHeadList(&DataList, &BD->listEntry);
	Status = STATUS_SUCCESS;

	KeReleaseInStackQueuedSpinLock(&LockHandle);
	return Status;
}

//移除BIND_DATA
NTSTATUS RemoveDataList(BIND_DATA_LIST *fd){
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PKLOCK_QUEUE_HANDLE lockHandle = NULL;

	KeAcquireInStackQueuedSpinLock(&DataListLock, lockHandle);

	RemoveEntryList(&fd->listEntry);
	Status = STATUS_SUCCESS;

	KeReleaseInStackQueuedSpinLock(lockHandle);
	return Status;
}


