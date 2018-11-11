#ifndef SSDT_HOOK_H
#define SSDT_HOOK_H


#include <ntddk.h>
#include <stdlib.h>

#define PAGEDCODE code_seg("PAGE")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("INIT")

#define PAGEDDATA data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("INIT")

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

#define MAX_PROCESS_ARRARY_LENGTH		1024

//定义 SSDT(系统服务描述表) 中服务个数的最大数目
//这里定义为 1024 个，实际上在 XP SP3 是 0x0128 个
#define MAX_SYSTEM_SERVICE_NUMBER 1024

//根据 Zw_ServiceFunction 获取 Zw_ServiceFunction 在 SSDT 中所对应的服务的索引号
//同： mov eax (索引号)   eax
#define SYSCALL_INDEX(ServiceFunction) \
	(*(PULONG)((PUCHAR)ServiceFunction + 1))

//声明NTKERNELAPI
NTKERNELAPI
NTSTATUS
PsLookupProcessByProcessId(HANDLE Id, PEPROCESS *Process);


//根据 Zw_ServiceFunction 来获得服务在 SSDT 中的索引号，
//然后再通过该索引号来获取 Nt_ServiceFunction的地址
//
#define SYSCALL_FUNCTION(ServiceFunction) \
	KeServiceDescriptorTable->ntoskrnl.ServiceTableBase[SYSCALL_INDEX(ServiceFunction)]

//定义SSDT结构
typedef struct _KSYSTEM_SERVICE_TABLE
{
	PULONG  ServiceTableBase;					// SSDT (System Service Dispatch Table)的基地址
	PULONG  ServiceCounterTableBase;			// 用于 checked builds, 包含 SSDT 中每个服务被调用的次数
	ULONG   NumberOfService;					// 服务函数的个数, NumberOfService * 4 就是整个地址表的大小
	ULONG   ParamTableBase;						// SSPT(System Service Parameter Table)的基地址

} KSYSTEM_SERVICE_TABLE, *PKSYSTEM_SERVICE_TABLE;


typedef struct _KSERVICE_TABLE_DESCRIPTOR
{
	KSYSTEM_SERVICE_TABLE   ntoskrnl;			// ntoskrnl.exe 的服务函数
	KSYSTEM_SERVICE_TABLE   win32k;				// win32k.sys 的服务函数(GDI32.dll/User32.dll 的内核支持)
	KSYSTEM_SERVICE_TABLE   notUsed1;
	KSYSTEM_SERVICE_TABLE   notUsed2;

} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;

//一些常用结构体的声明
typedef struct _SYSTEM_THREAD_INFORMATION
{
	LARGE_INTEGER	KernelTime;		//线程在内核执行时间
	LARGE_INTEGER	UserTime;		//线程在用户模式执行时间
	LARGE_INTEGER	CreateTime;		//线程创建时间
	ULONG			WaitTime;		//线程总的等待时间
	PVOID			StartAddress;	//线程起始地址
	CLIENT_ID		ClientId;		//进程和线程鉴定体
	KPRIORITY		Priority;		//线程优先级
	LONG			BasePriority;	//线程基础优先级
	ULONG			ContextSwitches;//由线程执行的上下文交换次数
	ULONG			ThreadState;	//当前线程状态
	ULONG			WaitReason;		//等待原因

} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION //win8中size = 184
{
	ULONG			NextEntryOffset;			//从输出缓冲区去下一个进程入口的偏移
	ULONG			NumberOfThreads;			//进程的线程数量
	LARGE_INTEGER	SpareLi1;					//Reserved[0]
	LARGE_INTEGER	SpareLi2;					//Reserved[1]
	LARGE_INTEGER	SpareLi3;					//Reserved[2]
	LARGE_INTEGER	CreateTime;					//进程创建时间,<=100ns
	LARGE_INTEGER	UserTime;					//用户模式的有效执行时间
	LARGE_INTEGER	KernelTime;					//内核模式的有效执行时间
	UNICODE_STRING	ImageName;					//进程名，基于可执行文件名
	KPRIORITY		BasePriority;				//进程起始优先
	HANDLE			UniqueProcessId;			//唯一的进程标识
	HANDLE			InheritedFromUniqueProcessId;	//创建者的标识
	ULONG			HandleCount;				//打开的句柄的数量
	ULONG			SessionId;
	ULONG_PTR		PageDirectoryBase;
	SIZE_T			PeakVirtualSize;
	SIZE_T			VirtualSize;
	ULONG			PageFaultCount;
	SIZE_T			PeakWorkingSetSize;
	SIZE_T			WorkingSetSize;
	SIZE_T			QuotaPeakPagedPoolUsage;
	SIZE_T			QuotaPagedPoolUsage;
	SIZE_T			QuotaPeakNonPagedPoolUsage;
	SIZE_T			QuotaNonPagedPoolUsage;
	SIZE_T			PagefileUsage;
	SIZE_T			PeakPagefileUsage;
	SIZE_T			PrivatePageCount;
	LARGE_INTEGER	ReadOperationCount;
	LARGE_INTEGER	WriteOperationCount;
	LARGE_INTEGER	OtherOperationCount;
	LARGE_INTEGER	ReadTransferCount;
	LARGE_INTEGER	WriteTransferCount;
	LARGE_INTEGER	OtherTransferCount;

} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemMirrorMemoryInformation,
	SystemPerformanceTraceInformation,
	SystemObsolete0,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemVerifierAddDriverInformation,
	SystemVerifierRemoveDriverInformation,
	SystemProcessorIdleInformation,
	SystemLegacyDriverInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation,
	SystemTimeSlipNotification,
	SystemSessionCreate,
	SystemSessionDetach,
	SystemSessionInformation,
	SystemRangeStartInformation,
	SystemVerifierInformation,
	SystemVerifierThunkExtend,
	SystemSessionProcessInformation,
	SystemLoadGdiDriverInSystemSpace,
	SystemNumaProcessorMap,
	SystemPrefetcherInformation,
	SystemExtendedProcessInformation,
	SystemRecommendedSharedDataAlignment,
	SystemComPlusPackage,
	SystemNumaAvailableMemory,
	SystemProcessorPowerInformation,
	SystemEmulationBasicInformation,
	SystemEmulationProcessorInformation,
	SystemExtendedHandleInformation,
	SystemLostDelayedWriteInformation,
	SystemBigPoolInformation,
	SystemSessionPoolTagInformation,
	SystemSessionMappedViewInformation,
	SystemHotpatchInformation,
	SystemObjectSecurityMode,
	SystemWatchdogTimerHandler,
	SystemWatchdogTimerInformation,
	SystemLogicalProcessorInformation,
	SystemWow64SharedInformation,
	SystemRegisterFirmwareTableInformationHandler,
	SystemFirmwareTableInformation,
	SystemModuleInformationEx,
	SystemVerifierTriageInformation,
	SystemSuperfetchInformation,
	SystemMemoryListInformation,
	SystemFileCacheInformationEx,
	MaxSystemInfoClass

} SYSTEM_INFORMATION_CLASS;


NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation(
	__in SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
	__in ULONG SystemInformationLength,
	__out_opt PULONG ReturnLength);

/*
表示NTTERMINATEPROCESS为一种指向函数的指针类型的名字。
要使用其，只需直接使用NTTERMINATEPROCESS即可
*/
typedef NTSTATUS (*NTQUERYSYSTEMINFORMATION)(SYSTEM_INFORMATION_CLASS,
	__out_bcount_opt(SystemInformationLength) PVOID,ULONG,__out_opt PULONG);

typedef NTSTATUS(*NTTERMINATEPROCESS)(HANDLE, NTSTATUS);

NTSTATUS  MyNtQuerySystemInformation(
	__in SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
	__in ULONG SystemInformationLength,
	__out_opt PULONG ReturnLength);

NTSTATUS MyNtTerminateProcess(
	__in_opt HANDLE ProcessHandle,
	__in NTSTATUS ExitStatus
	);

//定义一个指向固定函数的指针变量,用于保存老的函数地址
NTTERMINATEPROCESS pOldNtTerminateProcess;

NTQUERYSYSTEMINFORMATION pOldNtQuerySystemInformation;


//获得进程名(基于可执行文件的名字 xxx.exe)
PUCHAR PsGetProcessImageFileName(__in PEPROCESS Process);


ULONG g_PIDHideArray[MAX_PROCESS_ARRARY_LENGTH];//隐藏进程ID数组
ULONG g_PIDProtectArray[MAX_PROCESS_ARRARY_LENGTH];//保护进程ID数组

//隐藏的和受保护的数量
ULONG g_currHideArrayLen = 0;
ULONG g_currProtectArrayLen = 0;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,IN PUNICODE_STRING pRegistryPath);

NTSTATUS SSDTGeneralDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);//分发函数声明
NTSTATUS SSDTCreateDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS SSDTCloseDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS SSDTReadDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS SSDTWriteDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS SSDTDeviceIoControlDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);

VOID SSDTDriverUnload(IN PDRIVER_OBJECT pDriverObject);

//用来保存 SSDT 中所有的旧的服务函数的地址
ULONG oldSysServiceAddr[MAX_SYSTEM_SERVICE_NUMBER];

//禁止写入保护，也就是恢复到只读
VOID DisableWriteProtect(ULONG oldAttr);

//允许写入保护，也就是设置为可写
VOID EnableWriteProtect(PULONG pOldAttr);

//备份 SSDT 中所有系统服务的地址
VOID BackupSysServicesTable();

//安装 Hook
NTSTATUS InstallSSDTHook(ULONG oldService, ULONG newService);

//解除 Hook
NTSTATUS UnInstallSSDTHook(ULONG oldService);

//验证 uPID 所代表的进程是否存在于隐藏进程列表中，即判断 uPID 这个进程是否需要隐藏
ULONG ValidateProcessNeedHide(ULONG uPID);

//验证 uPID 所代表的进程是否存在于保护进程列表中，即判断 uPID 这个进程是否需要保护
ULONG ValidateProcessNeedProtect(ULONG uPID);

//往隐藏进程列表中插入 uPID
ULONG InsertHideProcess(ULONG uPID);

//从隐藏进程列表中移除 uPID
ULONG RemoveHideProcess(ULONG uPID);

//往保护进程列表中插入 uPID
ULONG InsertProtectProcess(ULONG uPID);

//从隐藏进程列表中移除 uPID
ULONG RemoveProtectProcess(ULONG uPID);

//注册进程回调函数
VOID CreateProcessNotifyEx(
__inout PEPROCESS  Process,
__in HANDLE  ProcessId,
__in_opt PPS_CREATE_NOTIFY_INFO  CreateInfo
);

#endif