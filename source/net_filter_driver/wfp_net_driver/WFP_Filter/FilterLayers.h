#pragma once

#include <initguid.h>
#include <ntddk.h>
#include "fwpmk.h"
#include "fwpsk.h"

#ifndef MAX_PATH
#define MAX_PATH	(260)
#endif

#define FILTER_MAJOR_NDIS_VERSION 6

#if defined(NDIS60)
#define FILTER_MINOR_NDIS_VERSION 0
#elif defined(NDIS620)
#define FILTER_MINOR_NDIS_VERSION 20
#elif defined(NDIS630)
#define FILTER_MINOR_NDIS_VERSION 30
#endif

#define PROCESSNAME_LENG 1024
#define MAX_NAME 16*4

#define WFP_DEVICE_NAME				L"\\Device\\wfp_device"
#define WFP_SYM_LINK_NAME			L"\\DosDevices\\wfp_device"

#define WFP_ESTABLISHED_CALLOUT_DISPLAY_NAME	L"WfpEstablishedCalloutName"
#define WFP_ESTABLISHED_SUB_LAYER_DISPLAY_NAME	L"WfpEstablishSubLayerName"
#define WFP_FILTER_ESTABLISH_DISPLAY_NAME		L"WfpFilterEstablishName"

#define WFP_DATA_CALLOUT_DISPLAY_NAME	L"WfpDataCalloutName"
#define	WFP_DATA_SUB_LAYER_DISPLAY_NAME	L"WfpDataSubLayerName"
#define WFP_FILTER_DATA_DISPLAY_NAME	L"WfpFilterDataName"

#define HTTP_DEFAULT_PORT	80


// {D969FC67-6FB2-4504-91CE-A97C3C32AD36}
DEFINE_GUID(WFP_ESTABLISHED_CALLOUT_V4_GUID,
	0xd969fc67, 0x6fb2, 0x4504, 0x91, 0xce, 0xa9, 0x7c, 0x3c, 0x32, 0xad, 0x36);

// {ED6A516A-36D1-4881-BCF0-ACEB4C04C21C}
DEFINE_GUID(WFP_ESTABLISHED_SUBLAYER_GUID,
	0xed6a516a, 0x36d1, 0x4881, 0xbc, 0xf0, 0xac, 0xeb, 0x4c, 0x4, 0xc2, 0x1c);

//  6422cfff2ec64f5cbc3c7ef1bcf04d08
DEFINE_GUID(WFP_DATA_CALLOUT_V4_GUID,
	0x6422cfff, 0x2ec6, 0x4f5c, 0xbc, 0x3c, 0x7e, 0xf1, 0xbc, 0xf0, 0x4d, 0x08);

//  a95914e08ac14fb78f274cca022bd1a3
DEFINE_GUID(WFP_DATA_SUBLAYER_GUID,
	0xa95914e0, 0x8ac1, 0x4fb7, 0x8f, 0x27, 0x4c, 0xca, 0x02, 0x2b, 0xd1, 0xa3);

typedef struct _FLOW_DATA
{
	LIST_ENTRY  listEntry;
	UINT64      flowHandle;
	UINT64      flowContext;
	UINT64      calloutId;
	ULONG       localAddressV4;
	USHORT      localPort;
	USHORT      ipProto;
	ULONG       remoteAddressV4;
	USHORT      remotePort;
	WCHAR*      processPath;
	UINT64	   processID;
	BOOLEAN     deleting;
} FLOW_DATA;

typedef struct _BIND_DATA_LIST{
	LIST_ENTRY  listEntry;		//链表
	ULONG       remoteAddressV4;//IP
	ULONG       localAddressV4;	//IP
	WCHAR	    wProcessPath[PROCESSNAME_LENG]; //进程全路径
	BYTE		imageName[MAX_NAME];			//进程名称
	UINT64	    uProcessID;	//进程ID
	UINT64       uDataLength;//数据长度
	USHORT      uRemotePort;//链接端口
	USHORT      uLocalPort;	//本地端口
	BOOLEAN	    bIsSend;	//是否是发送
}BIND_DATA_LIST, *PBIND_DATA_LIST;

typedef struct _TRANSFER_R3
{
	ULONG      RemoteIp;         //远端IP
	ULONG      SourceIp;         //源IP

	UINT64	   uProcessID;			//进程ID
	UINT64      uDataLength;			//数据长度
	USHORT     uRemotePort;			//链接端口
	USHORT     uLocalPort;			//本地端口
	BOOLEAN	   bIsSend;				//是否是发送
	WCHAR	   wProcessPath[PROCESSNAME_LENG]; //进程全路径
	BYTE		imageName[MAX_NAME];//进程名称
}TRANSFER_R3, *PTRANSFER_R3;

//App 2 Driver的规则库
typedef struct _RULE_FROM_R3{
	WCHAR ProcessPath[1024];
	BYTE  ImageName[64];
}RULE_FROM_R3, *PRULE_FROM_R3;

typedef struct _RULES_LIST{
	LIST_ENTRY   listEntry;
	RULE_FROM_R3 Rule;
}RULES_LIST, *PRULES_LIST;

NTSTATUS DriverEntry(__in struct _DRIVER_OBJECT* DriverObject, __in PUNICODE_STRING RegistryPath);

/*
Established 层函数声明：
*/

NTSTATUS WfpEstablishInit();

VOID WfpEstablishUnInit();

NTSTATUS InsertContextList(FLOW_DATA *fd);

NTSTATUS RemoveContextList(FLOW_DATA *fd);

NTSTATUS WfpEstablishRegisterCalloutImple(
								  IN OUT void* deviceObject,
								  IN  FWPS_CALLOUT_CLASSIFY_FN ClassifyFunction,
								  IN  FWPS_CALLOUT_NOTIFY_FN NotifyFunction,
								  IN  FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN FlowDeleteFunction,
								  IN  GUID const* calloutKey,
								  IN  UINT32 flags,
								  OUT UINT32* calloutId
								  );

NTSTATUS WfpEstablishRegisterCallouts( IN OUT void* deviceObject );

VOID NTAPI Established_ClassifyFn_V4(
	IN const FWPS_INCOMING_VALUES0  *inFixedValues,
	IN const FWPS_INCOMING_METADATA_VALUES0  *inMetaValues,
	IN OUT VOID  *layerData,
	IN OPTIONAL const void  *classifyContext,
	IN const FWPS_FILTER1  *filter,
	IN UINT64  flowContext,
	OUT FWPS_CLASSIFY_OUT0  *classifyOut
	);


NTSTATUS NTAPI Established_NotifyFn_V4( IN FWPS_CALLOUT_NOTIFY_TYPE notifyType, IN const GUID* filterKey, IN const FWPS_FILTER* filter);

VOID NTAPI Established_FlowDeleteFn_V4( IN UINT16 layerId, IN UINT32 calloutId, IN UINT64 flowContext );

NTSTATUS WfpEstablishAddCallouts();

NTSTATUS WfpEstablishRegisterCallouts(IN OUT void* deviceObject);

NTSTATUS WfpEstablishRegisterCalloutImple(
								 IN OUT void* deviceObject,
								 IN  FWPS_CALLOUT_CLASSIFY_FN ClassifyFunction,
								 IN  FWPS_CALLOUT_NOTIFY_FN NotifyFunction,
								 IN  FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN FlowDeleteFunction,
								 IN  GUID const* calloutKey,
								 IN  UINT32 flags,
								 OUT UINT32* calloutId
								 );

NTSTATUS WfpEstablishAddSubLayer();

NTSTATUS WfpEstablishAddFilters();


VOID WfpEstablishUnRegisterCallouts();

VOID WfpEstablishRemoveCallouts();

VOID WfpEstablishRemoveSubLayer();

VOID WfpEstablishRemoveFilters();


/*
Stream Data 层函数声明：
*/

NTSTATUS WfpDataInit();

VOID WfpDataUnInit();

NTSTATUS InsertDataList(BIND_DATA_LIST *fd);

NTSTATUS RemoveDataList(BIND_DATA_LIST *fd);

NTSTATUS WfpDataRegisterCallouts();

NTSTATUS WfpDataAddCallouts();

NTSTATUS WfpDataAddSubLayer();

NTSTATUS WfpDataAddFilters();

VOID WfpDataRemoveCallouts();

VOID WfpDataRemoveSubLayer();

VOID WfpDataRemoveFilters();

VOID WfpDataUnRegisterCallouts();

ULONG GetNBLLength(PNET_BUFFER_LIST NBL);

VOID NTAPI Data_ClassifyFn_V4(
	IN const FWPS_INCOMING_VALUES  *inFixedValues,
	IN const FWPS_INCOMING_METADATA_VALUES  *inMetaValues,
	IN OUT VOID  *layerData,
	IN OPTIONAL const void  *classifyContext,
	IN const FWPS_FILTER1  *filter,
	IN UINT64  flowContext,
	OUT FWPS_CLASSIFY_OUT  *classifyOut
	);

NTSTATUS NTAPI Data_NotifyFn_V4(
	IN  FWPS_CALLOUT_NOTIFY_TYPE        notifyType,
	IN  const GUID*             filterKey,
	IN  const FWPS_FILTER*     filter);

VOID NTAPI Data_FlowDeleteFn_V4(
	IN UINT16  layerId,
	IN UINT32  calloutId,
	IN UINT64  flowContext
	);


/*主要函数原型声明*/
void  DriverUnload(__in struct _DRIVER_OBJECT  *DriverObject);

PDEVICE_OBJECT	CreateDevice(__in struct _DRIVER_OBJECT* DriverObject);

NTSTATUS IRPDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

void OpenEngine();

void CloseEngine();

NTSTATUS InitWfp();

VOID UninitWfp();

VOID DeleteDevice();

/*IoCtrl 与应用层交互的函数：*/
NTKERNELAPI
NTSTATUS
PsLookupProcessByProcessId(HANDLE Id,PEPROCESS *Process);
NTKERNELAPI
UCHAR*
PsGetProcessImageFileName(IN PEPROCESS Process);

NTSTATUS TransferControl(PDEVICE_OBJECT DeviceObject,PIRP Irp,int * Info);

VOID MonitorData(PTRANSFER_R3 tr);

PEPROCESS LookupProcess(HANDLE Pid);


//Rule 相关函数声明
VOID WfpRuleListInit();

VOID WfpRuleListUnInit();

NTSTATUS AddRuleFromR3(PRULE_FROM_R3 pRFR);

NTSTATUS RemoveRule(PRULE_FROM_R3 prfr);

NTSTATUS InsertRuleList(PRULES_LIST rl);

NTSTATUS Data_Filtering(PBIND_DATA_LIST pBDL);
