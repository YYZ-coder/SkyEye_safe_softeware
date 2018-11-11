#include "FilterLayers.h"

//ID
UINT32	g_uFwpsEstablishedCallOutId = 0;
UINT32	g_uFwpmEstablishedCallOutId = 0;
UINT64	g_uEstablishedFilterId = 0;

//FlowContext List
extern LIST_ENTRY  FlowContextList;
extern KSPIN_LOCK  FlowContextListLock;

extern PDEVICE_OBJECT g_pDeviceObj;
//Engine
extern HANDLE	g_hEngine;

//Stream Data
extern UINT32	g_uFwpsDataCallOutId;

NTSTATUS WfpEstablishInit(){

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	do{
		if (STATUS_SUCCESS != WfpEstablishRegisterCallouts(g_pDeviceObj))
			break;

		if (STATUS_SUCCESS != WfpEstablishAddCallouts())
			break;

		if (STATUS_SUCCESS != WfpEstablishAddSubLayer())
			break;

		if (STATUS_SUCCESS != WfpEstablishAddFilters())
			break;

		Status = STATUS_SUCCESS;
	} while (FALSE);
	return Status;
}

VOID WfpEstablishUnInit(){
	WfpEstablishRemoveFilters();
	WfpEstablishRemoveSubLayer();
	WfpEstablishRemoveCallouts();
	WfpEstablishUnRegisterCallouts();

	KdPrint(("Establish List UnInit\n"));
	PLIST_ENTRY entry;
	KLOCK_QUEUE_HANDLE handle;

	if (&FlowContextList == NULL)
		return;

	if (IsListEmpty(&FlowContextList))
		return;

	KeAcquireInStackQueuedSpinLock(&FlowContextListLock,&handle);
	//清空链表
	while (!IsListEmpty(&FlowContextList)){
		entry = RemoveTailList(&FlowContextList);
		FLOW_DATA* data = CONTAINING_RECORD(entry, FLOW_DATA, listEntry);
		ExFreePool(data);
	}
	KeReleaseInStackQueuedSpinLock(&handle);
}

//向驱动设备注册FWPS_CALLOUT
NTSTATUS WfpEstablishRegisterCalloutImple(IN OUT void* deviceObject, IN  FWPS_CALLOUT_CLASSIFY_FN ClassifyFunction,
	IN  FWPS_CALLOUT_NOTIFY_FN NotifyFunction, IN  FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN FlowDeleteFunction,
	IN  GUID const* calloutKey, IN  UINT32 flags, OUT UINT32* calloutId)
{
	FWPS_CALLOUT sCallout;
	NTSTATUS status = STATUS_SUCCESS;

	memset(&sCallout, 0, sizeof(FWPS_CALLOUT));

	sCallout.calloutKey = *calloutKey;
	sCallout.flags = flags;
	sCallout.classifyFn = ClassifyFunction;
	sCallout.notifyFn = NotifyFunction;
	sCallout.flowDeleteFn = FlowDeleteFunction;

	status = FwpsCalloutRegister(deviceObject, &sCallout, calloutId);

	return status;
}

//注册FWPS_CALLOUT
NTSTATUS WfpEstablishRegisterCallouts(IN OUT void* deviceObject)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	do
	{
		if (deviceObject == NULL)
			break;

		status = WfpEstablishRegisterCalloutImple(deviceObject,
			Established_ClassifyFn_V4,
			Established_NotifyFn_V4,
			Established_FlowDeleteFn_V4,
			&WFP_ESTABLISHED_CALLOUT_V4_GUID,
			0,
			&g_uFwpsEstablishedCallOutId);
		if (status != STATUS_SUCCESS)
		{
			break;
		}
		status = STATUS_SUCCESS;
	} while (FALSE);
	return status;
}

//为Engine添加FWPM_CALLOUT 
NTSTATUS WfpEstablishAddCallouts()
{
	NTSTATUS status = STATUS_SUCCESS;
	FWPM_CALLOUT fwpmCallout = { 0 };
	fwpmCallout.flags = 0;
	do
	{
		if (g_hEngine == NULL)
		{
			break;
		}
		//displayData：为了给出FWPM_CALLOUT的名字和详细信息
		fwpmCallout.displayData.name = (wchar_t *)WFP_ESTABLISHED_CALLOUT_DISPLAY_NAME;
		fwpmCallout.displayData.description = (wchar_t *)WFP_ESTABLISHED_CALLOUT_DISPLAY_NAME;

		//uniquely identifies：必须与FWPS_CALLOUT的ID一样
		fwpmCallout.calloutKey = WFP_ESTABLISHED_CALLOUT_V4_GUID;

		//applcableLayer：filtering layer at which the callout is applicable.
		fwpmCallout.applicableLayer = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4;//FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4;

		//向g_hEngine中加入
		status = FwpmCalloutAdd(g_hEngine, &fwpmCallout, NULL, &g_uFwpmEstablishedCallOutId);

		if (!NT_SUCCESS(status) && (status != STATUS_FWP_ALREADY_EXISTS))
		{
			break;
		}
		status = STATUS_SUCCESS;
	} while (FALSE);
	return status;
}

//为Engine添加子层
NTSTATUS WfpEstablishAddSubLayer()
{
	KdPrint(("WfpEstablish AddSubLayer\n"));

	NTSTATUS nStatus = STATUS_UNSUCCESSFUL;
	FWPM_SUBLAYER SubLayer = { 0 };
	SubLayer.flags = 0;
	SubLayer.displayData.description = WFP_ESTABLISHED_SUB_LAYER_DISPLAY_NAME;
	SubLayer.displayData.name = WFP_ESTABLISHED_SUB_LAYER_DISPLAY_NAME;
	SubLayer.subLayerKey = WFP_ESTABLISHED_SUBLAYER_GUID;
	SubLayer.weight = 65535;
	if (g_hEngine != NULL)
	{
		nStatus = FwpmSubLayerAdd(g_hEngine, &SubLayer, NULL);
	}
	return nStatus;

}

//为Engine添加过滤器
NTSTATUS WfpEstablishAddFilters()
{
	KdPrint(("WfpEstablish AddFilters\n"));

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
		Filter.displayData.description = WFP_FILTER_ESTABLISH_DISPLAY_NAME;
		Filter.displayData.name = WFP_FILTER_ESTABLISH_DISPLAY_NAME;
		Filter.flags = 0;
		Filter.layerKey = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4;
		Filter.subLayerKey = WFP_ESTABLISHED_SUBLAYER_GUID;
		Filter.weight.type = FWP_EMPTY;
		Filter.numFilterConditions = 1;
		Filter.filterCondition = FilterCondition;
		Filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
		Filter.action.calloutKey = WFP_ESTABLISHED_CALLOUT_V4_GUID;

		FilterCondition[0].fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
		FilterCondition[0].matchType = FWP_MATCH_EQUAL;
		FilterCondition[0].conditionValue.type = FWP_V4_ADDR_MASK;
		FilterCondition[0].conditionValue.v4AddrMask = &AddrAndMask;

		nStatus = FwpmFilterAdd(g_hEngine, &Filter, NULL, &g_uEstablishedFilterId);
		if (STATUS_SUCCESS != nStatus)
		{
			break;
		}
		nStatus = STATUS_SUCCESS;
	} while (FALSE);
	return nStatus;
}

//移除Establish layer 的FWPM_CALLOUT
VOID WfpEstablishRemoveCallouts()
{
	KdPrint(("WfpEstablish RemoveCallouts\n"));

	if (g_hEngine != NULL)
	{
		FwpmCalloutDeleteById(g_hEngine, g_uFwpmEstablishedCallOutId);
		g_uFwpmEstablishedCallOutId = 0;
	}

}

//移除Establish layer 的SubLayer
VOID WfpEstablishRemoveSubLayer()
{
	KdPrint(("WfpEstablish RemoveSubLayer\n"));

	if (g_hEngine != NULL)
	{
		FwpmSubLayerDeleteByKey(g_hEngine, &WFP_ESTABLISHED_SUBLAYER_GUID);
	}
}

//移除Establish layer 的Filter
VOID WfpEstablishRemoveFilters()
{
	KdPrint(("WfpEstablish RemoveFilters\n"));

	if (g_hEngine != NULL)
	{
		FwpmFilterDeleteById(g_hEngine, g_uEstablishedFilterId);
	}
}

//UnRegister FWPS_CALLOUT
VOID WfpEstablishUnRegisterCallouts()
{
	KdPrint(("WfpEstablish UnRegisterCallouts\n"));

	FwpsCalloutUnregisterById(g_uFwpsEstablishedCallOutId);
	g_uFwpsEstablishedCallOutId = 0;
}

//inFixedValue 包含双方IP,端口等固定信息；
//inMetaValue  包含一些发起的应用层ID，进程路径等信息；
//flowContext  包含上下文信息，可以存储传输中的公共信息，在其它层过滤时可访问。
VOID NTAPI Established_ClassifyFn_V4(
	IN const FWPS_INCOMING_VALUES  *inFixedValues,IN const FWPS_INCOMING_METADATA_VALUES  *inMetaValues,
	IN OUT VOID  *layerData,IN OPTIONAL const void  *classifyContext,IN const FWPS_FILTER1  *filter,IN UINT64  flowContext,
	OUT FWPS_CLASSIFY_OUT  *classifyOut)
{
	//KdPrint(("Wfp_Established_ClassifyFn_V4\n"));

	WORD	wDirection = 0;
	WORD	wRemotePort = 0;
	WORD	wSrcPort = 0;
	WORD	wProtocol = 0;
	ULONG	ulSrcIPAddress = 0;
	ULONG	ulRemoteIPAddress = 0;

	UINT64	proID;
	FWP_BYTE_BLOB *proPath = NULL;

	ULONG	Length = 0;

	FLOW_DATA *FlowContext = NULL;

	NTSTATUS	Status = STATUS_SUCCESS;

	FlowContext = ExAllocatePoolWithTag(NonPagedPool,sizeof(FLOW_DATA),'FC');

	do{
		if (!FWPS_IS_METADATA_FIELD_PRESENT(inMetaValues, FWPS_METADATA_FIELD_PROCESS_PATH)){
			Status = STATUS_NOT_FOUND;
			break;
		}

		if (!(classifyOut->rights & FWPS_RIGHT_ACTION_WRITE))
			return;

		FlowContext->deleting = FALSE;

		//获得进程路径
		proPath = inMetaValues->processPath;
		FlowContext->processPath = ExAllocatePoolWithTag(NonPagedPool, proPath->size, 'pp');
		memcpy(FlowContext->processPath, proPath->data, proPath->size);

		//获得进程ID
		proID = inMetaValues->processId;
		FlowContext->processID = proID;

		//wDirection表示数据包的方向,取值为	//FWP_DIRECTION_INBOUND/FWP_DIRECTION_OUTBOUND
		wDirection = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_DIRECTION].value.int8;

		//wSrcPort表示本地端口，主机序
		wSrcPort = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_PORT].value.uint16;
		FlowContext->localPort = wSrcPort;

		//wRemotePort表示远端端口，主机序
		wRemotePort = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_PORT].value.uint16;
		FlowContext->remotePort = wRemotePort;

		//ulSrcIPAddress 表示源IP
		ulSrcIPAddress = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_ADDRESS].value.uint32;
		FlowContext->localAddressV4 = ulSrcIPAddress;

		//ulRemoteIPAddress 表示远端IP
		ulRemoteIPAddress = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_ADDRESS].value.uint32;
		FlowContext->remoteAddressV4 = ulRemoteIPAddress;

		//wProtocol表示网络协议，可以取值是IPPROTO_ICMP/IPPROTO_UDP/IPPROTO_TCP
		wProtocol = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_PROTOCOL].value.uint8;

		if (wDirection == FWP_DIRECTION_INBOUND){
			KdPrint(("---IN BOUND\n"));
		}
		else if (wDirection == FWP_DIRECTION_OUTBOUND){
			KdPrint(("---OUT BOUND\n"));
		}

		//Establish layer Connect Stream layer
		UINT64 FlowHandle = inMetaValues->flowHandle;
		FlowContext->flowHandle = inMetaValues->flowHandle;

		//Insert ContextList
		Status = InsertContextList(FlowContext);
		if (!NT_SUCCESS(Status)){
			//KdPrint(("Establish InsertContextList Unsuccessful!\n"));
			classifyOut->actionType = FWP_ACTION_CONTINUE;
			break;
		}

		//AssocaiateContext
		Status = FwpsFlowAssociateContext(FlowHandle,
										FWPS_LAYER_STREAM_V4,
										g_uFwpsDataCallOutId,
										(UINT64)FlowContext);
		if (!NT_SUCCESS(Status)){
			classifyOut->actionType = FWP_ACTION_CONTINUE;
			break;
		}

		classifyOut->actionType = FWP_ACTION_PERMIT;
	
	} while (FALSE);

	if (!NT_SUCCESS(Status)){
		if (FlowContext->processPath)
			ExFreePoolWithTag(FlowContext->processPath,'pp');
		ExFreePoolWithTag(FlowContext,'FC');
	}

	//clear FWPS_RIGHT_ACTION_WRITE Tag
	if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT)
	{
		classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
	}
	return;
}

NTSTATUS NTAPI Established_NotifyFn_V4(
	IN  FWPS_CALLOUT_NOTIFY_TYPE notifyType,IN  const GUID* filterKey,IN  const FWPS_FILTER* filter)
{
	return STATUS_SUCCESS;
}

VOID NTAPI Established_FlowDeleteFn_V4(
	IN UINT16  layerId,IN UINT32  calloutId,IN UINT64  flowContext)
{

}

//插入FlowContextList
NTSTATUS InsertContextList(FLOW_DATA *fd){
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	KLOCK_QUEUE_HANDLE LockHandle;

	KeAcquireInStackQueuedSpinLock(&FlowContextListLock, &LockHandle);

	InsertHeadList(&FlowContextList, &fd->listEntry);
	Status = STATUS_SUCCESS;

	KeReleaseInStackQueuedSpinLock(&LockHandle);
	return Status;
}

//移除FlowContext
NTSTATUS RemoveContextList(FLOW_DATA *fd){
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PKLOCK_QUEUE_HANDLE lockHandle = NULL;

	if (IsListEmpty(&FlowContextList))
		return STATUS_SUCCESS;

	KeAcquireInStackQueuedSpinLock(&FlowContextListLock, lockHandle);

	RemoveEntryList(&fd->listEntry);
	Status = STATUS_SUCCESS;

	KeReleaseInStackQueuedSpinLock(lockHandle);
	return Status;
}
