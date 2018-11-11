/* * * * * * * * * * * * * * * * * * * 

利用 PsSetCreateProcessNotifyRoutineEx
实现对进程的监控(创建，关闭)

* * * * * * * * * * * * * * * * * * * */

#include "Regedit_Operation.h"
#include "ProcessForbid.h"
#include "SSDT_HOOK.h"

//SkyEye发送SSDT当前进程ID并保护
#define IOCTL_SSDT_ADD \
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x830, METHOD_BUFFERED, \
	FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//SkyEye删除SSDT当前受保护进程ID
#define IOCTL_SSDT_DELETE \
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x831, METHOD_BUFFERED, \
	FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//发送用户输入的用户名,密码,DBname,DBhost：CP->SSDT
#define IOCTL_SSDT_SEND_UP\
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x850, METHOD_BUFFERED, \
	FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//获得当前账户用户名,密码,DBname,DBhost：Register+CP -> SkyEye
#define IOCTL_SSDT_GET_UP\
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x851, METHOD_BUFFERED, \
	FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//将安装信息写入注册表：SkyEye->Register
#define IOCTL_SSDT_SET_REG\
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN,\
	0x852,METHOD_BUFFERED,\
	FILE_WRITE_ACCESS | FILE_READ_ACCESS)

//从注册表读出安装信息：Register->CP
#define IOCTL_SSDT_GET_REG\
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN,\
	0x853,METHOD_BUFFERED,\
	FILE_WRITE_ACCESS | FILE_READ_ACCESS)

//清除驱动黑名单
#define IOCTL_SSDT_CLAER_BLACK_ILST\
	CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x832, METHOD_BUFFERED, \
	FILE_WRITE_ACCESS | FILE_READ_ACCESS)

#define DEVICE_NAME_PROCESS				L"\\Device\\SSDTProcess"
#define SYMBOLINK_NAME_PROCESS			L"\\??\\SSDTProcess"

//导出由 ntoskrnl.exe 所导出的 SSDT
extern PKSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTable;

//存储CP发来的用户名,密码
typedef struct _USER_PASSWORD{
	int  userLength;
	int  passwordLength;
	CHAR USER[10];
	CHAR PASSWORD[20];
}USER_PASSWORD, *PUSER_PASSWORD;

//用户记录链表
typedef struct _USER_LIST{
	LIST_ENTRY listEntry;
	USER_PASSWORD UserPassword;
}USER_LIST, *PUSER_LIST;
//记录链表的锁
KSPIN_LOCK userLock;
//用户链表
LIST_ENTRY userList;
//CP的信息+注册表的信息的数据结构发给SkyEye
PCP_TO_R3 pc2r = NULL;
//CP读取注册表的信息
PREG_TO_CP pRTC = NULL;
//标志变量，默认为FALSE
//为TRUE时，认为是SkyEye获得信息;
//为FALSE时，认为是CP获得信息.
BOOLEAN SkyEyeGetReg = FALSE;
//禁止打开的进程链
LIST_ENTRY ForbidProcessList;
//进程链的锁
KSPIN_LOCK ForbidProcessListLock;

//要传进去的禁止进程名和字符串长度
typedef struct _ForbidProcessName{
	int length;
	WCHAR ProcessName[50];
}ForbidProcessName, *PForbidProcessName;


//插入登陆的用户记录链表
NTSTATUS InsertUserList(PUSER_LIST up){
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	KLOCK_QUEUE_HANDLE handle;
	KeAcquireInStackQueuedSpinLock(&userLock, &handle);

	InsertHeadList(&userList, &up->listEntry);
	Status = STATUS_SUCCESS;

	KeReleaseInStackQueuedSpinLock(&handle);
	return Status;
}

//删除登陆的用户记录
NTSTATUS RemoveUserList(PUSER_LIST ul){
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	KLOCK_QUEUE_HANDLE handle;
	KeAcquireInStackQueuedSpinLock(&userLock, &handle);

	RemoveEntryList(&ul->listEntry);
	Status = STATUS_SUCCESS;

	KeReleaseInStackQueuedSpinLock(&handle);
	return Status;
}

//创建进程的回调函数
VOID CreateProcessNotifyEx(__inout PEPROCESS  Process,__in HANDLE  ProcessId,__in_opt PPS_CREATE_NOTIFY_INFO  CreateInfo){
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(Process);
	KdPrint(("SSDT-------Notify-In------->"));
	PWCHAR pSub = NULL;
	UNICODE_STRING  uniPath = { 0 };
	uniPath.Length = 0;
	uniPath.MaximumLength = 256 * 2;
	uniPath.Buffer = (PWSTR)ExAllocatePool(NonPagedPool, uniPath.MaximumLength);
	ULONG currenPID = (ULONG)ProcessId;
	//为NULL表示进程退出
	if (NULL == CreateInfo){
		/*for (int i = 0; i < MAX_PROCESS_ARRARY_LENGTH; i++){
			if (g_PIDProtectArray[i] == currenPID)
			{
				//获得当前进程全路径
				GetProcessPath(currenPID,&uniPath);
				//自己创建一个PPS_CREATE_NOTIFY_INFO结构，并返回
				PPS_CREATE_NOTIFY_INFO pPCNI = (PPS_CREATE_NOTIFY_INFO)
					ExAllocatePool(NonPagedPool, sizeof(PS_CREATE_NOTIFY_INFO));
				pPCNI->ImageFileName = &uniPath;
				pPCNI->ParentProcessId = PsGetProcessId(Process);
				PEPROCESS peProcess = NULL;
				if (NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &peProcess))){
					pPCNI->CreatingThreadId = ProcessId
				}
				ExFreePool(pPCNI);
				
			}
		}*/
			//g_PIDProtectArray
		KdPrint(("SSDT------process exits.\n"));
		return;
	}

	KdPrint(("SSDT------process create:(%wZ).\n", CreateInfo->ImageFileName));
	
	LIST_ENTRY *entry = NULL;
	if (!IsListEmpty(&ForbidProcessList)){
		for (entry = ForbidProcessList.Flink;
			entry != &ForbidProcessList;
			entry = entry->Blink){

			PForbidProcess data1 = CONTAINING_RECORD(entry, ForbidProcess, listEntry);
			KdPrint(("SSDT------deleteProcessName in list:%ws\n", data1->ProcessName));
			pSub = wcswcs(CreateInfo->ImageFileName->Buffer, data1->ProcessName);
			if (pSub != NULL)
				break;
		}
	}
	if (NULL != pSub)
	{
		//修改返回结果为拒绝访问，使得创建进程失败
		CreateInfo->CreationStatus = STATUS_ACCESS_DENIED;
	}

	return;
}

//#pragma INITCODE
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,IN PUNICODE_STRING pRegistryPath)
{
	//用来躲避windows的驱动检测,DriverSection表示系统将驱动呈链状的当前驱动在其中的节点
#ifdef _AMD64_ 
	DbgPrint("64\r\n");
	*((PCHAR)pDriverObject->DriverSection + 0x68) |= 0x20;
#else
	DbgPrint("32\r\n");
	*((PCHAR)pDriverObject->DriverSection + 0x34) |= 0x20;
#endif//_AMD_64


	ULONG i;
	NTSTATUS status;
	UNICODE_STRING strDeviceName;
	UNICODE_STRING strSymbolLinkName;
	PDEVICE_OBJECT pDeviceObject;

	pDeviceObject = NULL;

	KdPrint(("register: %ws\n",pRegistryPath->Buffer));

	RtlInitUnicodeString(&strDeviceName, DEVICE_NAME_PROCESS);
	RtlInitUnicodeString(&strSymbolLinkName, SYMBOLINK_NAME_PROCESS);

	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = SSDTGeneralDispatcher;
	}

	pDriverObject->MajorFunction[IRP_MJ_CREATE] = SSDTCreateDispatcher;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = SSDTCloseDispatcher;
	pDriverObject->MajorFunction[IRP_MJ_READ] = SSDTReadDispatcher;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = SSDTWriteDispatcher;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SSDTDeviceIoControlDispatcher;

	pDriverObject->DriverUnload = SSDTDriverUnload;

	status = IoCreateDevice(pDriverObject, 0, &strDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("Create Device Failed!\n"));
		return status;
	}
	if (!pDeviceObject)
	{
		return STATUS_UNEXPECTED_IO_ERROR;
	}

	KdPrint(("DriverEntry===->\n"));

	//使用直接 IO 读写方式
	pDeviceObject->Flags |= DO_DIRECT_IO;
	pDeviceObject->AlignmentRequirement = FILE_WORD_ALIGNMENT;
	status = IoCreateSymbolicLink(&strSymbolLinkName, &strDeviceName);

	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	//初始化用户记录链表
	InitializeListHead(&userList);
	//初始化用户记录锁
	KeInitializeSpinLock(&userLock);
	//初始化禁止打开的进程链
	InitializeListHead(&ForbidProcessList);
	//初始化禁止打开进程链的锁
	KeInitializeSpinLock(&ForbidProcessListLock);

	//初始化目前登陆的用户信息对象
	pc2r = (PCP_TO_R3)ExAllocatePool(NonPagedPool,sizeof(CP_TO_R3));
	if (pc2r != NULL)
		KdPrint(("pc2r Allocate Success!"));
	else
		KdPrint(("pc2r Allocate Failed!"));
	RtlZeroMemory(pc2r,sizeof(CP_TO_R3));
	for (int i = 0; i < 10; i++){
		pc2r->DBNAME[i] = '#';
		pc2r->HOST[i] = '#';
		pc2r->USER[i] = '#';
	}
	for (int j = 0; j < 20; j++)
		pc2r->PASSWORD[j] = '#';
	//初始化要发送给CP的数据库和系统信息
	pRTC = (PREG_TO_CP)ExAllocatePool(NonPagedPool, sizeof(REG_TO_CP));
	if (pRTC != NULL)
		KdPrint(("pRTC Allocate Success!"));
	else
		KdPrint(("pRTC Allocate Failed!"));
	RtlZeroMemory(pRTC, sizeof(REG_TO_CP));
	for (int i = 0; i < 10; i++){
		pRTC->dbHost[i] = '#';
		pRTC->dbName[i] = '#';
		pRTC->sysAdmin[i] = '#';
	}
	for (int i = 0; i < 20; i++)
		pRTC->sysPassword[i] = '#';

	//首先需要备份原来的 SSDT 系统服务描述表中所有服务的地址，这些地址主要用于实现解除 Hook
	BackupSysServicesTable();
	//设置进程创建，关闭回调函数
	NTSTATUS pn = STATUS_SUCCESS;
	pn = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)CreateProcessNotifyEx,FALSE);
	if (pn != STATUS_SUCCESS)
		KdPrint(("SSDT--------PsSetCreateProcessNotifyRoutineEx Successful!!!!"));
	else
		KdPrint(("SSDT--------Failed PsSetCreateProcessNotifyRoutineEx!!!!!"));

	//安装 Hook:用新的MyNtQuerySystemInformation/MyNtTerminateProcess替换ZwQuerySystemInformation/ZwTerminateProcess
	InstallSSDTHook((ULONG)ZwQuerySystemInformation, (ULONG)MyNtQuerySystemInformation);
	InstallSSDTHook((ULONG)ZwTerminateProcess, (ULONG)MyNtTerminateProcess);

	return STATUS_SUCCESS;
}


//用来去掉内存的可写属性，从而实现内存只读
VOID DisableWriteProtect(ULONG oldAttr)
{
	KdPrint(("DisableWriteProtect===->\n"));
	_asm
	{
		mov eax, oldAttr
		mov cr0, eax
		sti;
	}
}

//用来去掉内存的只读保护，从而实现可以写内存
VOID EnableWriteProtect(PULONG pOldAttr)
{
	KdPrint(("EnableWriteProtect===->\n"));
	ULONG uAttr;
	//汇编指令：
	_asm
	{
		cli;
		mov  eax, cr0;
		mov  uAttr, eax;
		and  eax, 0FFFEFFFFh; // CR0 16 BIT = 0 
		mov  cr0, eax;
	};

	//保存原有的 CRO 属性 
	*pOldAttr = uAttr;
}

//备份 SSDT 中原有服务的地址，因为我们在解除 Hook 时需要还原 SSDT 中原有地址
VOID BackupSysServicesTable()
{
	KdPrint(("BackupSysServicesTable===->\n"));
	ULONG i;
	//循环操作：从第0个索引开始，直到SSDT的ntoskrnl基地址最后，遍历所有的服务，并存储到一个结构中。
	for (i = 0; (i < KeServiceDescriptorTable->ntoskrnl.NumberOfService) && (i < MAX_SYSTEM_SERVICE_NUMBER); i++)
	{
		oldSysServiceAddr[i] = KeServiceDescriptorTable->ntoskrnl.ServiceTableBase[i];

		//oldSysServiceAddr[i] = *(PULONG)((ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTableBase + 4 * i);

		KdPrint(("\nBackupSysServicesTable - Function Information { Number: 0x%04X , Address: %08X}",
			i, oldSysServiceAddr[i]));
	}
}

//
//Mark:	其中的EnableWriteProtect和DisableWriteProtect相当于一个自旋锁的上锁和解锁
//开启HOOK
NTSTATUS InstallSSDTHook(ULONG oldService, ULONG newService)
{
	KdPrint(("InstallSSDTHook===->\n"));

	ULONG uOldAttr = 0;

	//实现可写，破解SSDT只读操作
	EnableWriteProtect(&uOldAttr);

	//给oldService的老地址处存上newService
	SYSCALL_FUNCTION(oldService) = newService;
	
	//恢复原状
	DisableWriteProtect(uOldAttr);

	return STATUS_SUCCESS;
}

//
//Mark:	其中的EnableWriteProtect和DisableWriteProtect相当于一个自旋锁的上锁和解锁
//关闭HOOK
NTSTATUS UnInstallSSDTHook(ULONG oldService)
{
	KdPrint(("UnInstallSSDTHook===->\n"));
	ULONG uOldAttr = 0;

	//实现可写
	EnableWriteProtect(&uOldAttr);

	//将原地址重新存放oldService
	SYSCALL_FUNCTION(oldService) = oldSysServiceAddr[SYSCALL_INDEX(oldService)];
	//实现可读
	DisableWriteProtect(uOldAttr);

	return STATUS_SUCCESS;
}


NTSTATUS SSDTGeneralDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}


NTSTATUS SSDTCreateDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	KdPrint(("Create"));
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


NTSTATUS SSDTCloseDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


NTSTATUS SSDTReadDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	NTSTATUS rtStatus;

	rtStatus = STATUS_NOT_SUPPORTED;

	return rtStatus;
}


NTSTATUS SSDTWriteDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	NTSTATUS rtStatus;

	rtStatus = STATUS_NOT_SUPPORTED;

	return rtStatus;
}

//清空链表
VOID clear(){
	KLOCK_QUEUE_HANDLE userhandle;
	KeAcquireInStackQueuedSpinLock(&userLock,&userhandle);
	//清空用户记录链表
	if (!IsListEmpty(&userList)){
		while (!IsListEmpty(&userList)){
			PLIST_ENTRY pEntry = RemoveTailList(&userList);
			PUSER_LIST data = CONTAINING_RECORD(pEntry, USER_LIST, listEntry);
			ExFreePool(data);
		}
	}
	KeReleaseInStackQueuedSpinLock(&userhandle);

	KLOCK_QUEUE_HANDLE handle;
	KeAcquireInStackQueuedSpinLock(&ForbidProcessListLock, &handle);
	//清空记录禁止进程的链表
	if (!IsListEmpty(&ForbidProcessList)){
		while (!IsListEmpty(&ForbidProcessList)){
			PLIST_ENTRY pEntry = RemoveTailList(&ForbidProcessList);
			PForbidProcess data = CONTAINING_RECORD(pEntry, ForbidProcess, listEntry);
			ExFreePool(data);
		}
	}
	KeReleaseInStackQueuedSpinLock(&handle);
}

//控制分发函数
NTSTATUS SSDTDeviceIoControlDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	KdPrint(("IOCtrl Dispatch"));
	NTSTATUS rtStatus = STATUS_SUCCESS;
	ULONG uPID = 0;
	ULONG uInLen;
	ULONG uOutLen;
	ULONG uCtrlCode;
	PIO_STACK_LOCATION pStack;
	//SkyEye => SSDT IOCTL_SSDT_ADD/IOCTL_SSDT_DELETE
	PULONG pInBuffer = NULL;
	//CP  =>  SSDT并存储 IOCTL_SSDT_SEND_UP
	PUSER_PASSWORD pUP = NULL;
	PUSER_LIST userList = NULL;
	//注册表&SSDT  => SkyEye IOCTL_SSDT_GET_UP
	PCP_TO_R3 pcp2r3 = NULL;
	//SkyEye  => 注册表  IOCTL_SSDT_SET_REG
	PR3_TO_REG pRTR = NULL;
	//注册表 => CP  IOCTL_SSDT_GET_REG
	PREG_TO_CP pRTCC = NULL;
	//删除的禁止进程名
	PForbidProcessName addProcesName = NULL;
	//删除的禁止进程名
	PForbidProcessName deleteProcesName = NULL;

	pStack = IoGetCurrentIrpStackLocation(pIrp);//得到当前IO堆栈
	uInLen = pStack->Parameters.DeviceIoControl.InputBufferLength;//得到输入缓冲区大小
	uOutLen = pStack->Parameters.DeviceIoControl.OutputBufferLength;//得到输出缓冲区大小
	uCtrlCode = pStack->Parameters.DeviceIoControl.IoControlCode;//得到IOCTL码

	switch (uCtrlCode){
	case IOCTL_SSDT_ADD://--------------SkyEye给当前进程加保护
		KdPrint(("----------Add----------"));
		pInBuffer = (PULONG)pIrp->AssociatedIrp.SystemBuffer;
		//插入被保护进程
		InsertProtectProcess(*pInBuffer);
		//插入被隐藏进程
		InsertHideProcess(*pInBuffer);
		pIrp->IoStatus.Status = rtStatus;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		break;
	case IOCTL_SSDT_DELETE://--------------SKyEye去除当前进程保护
		KdPrint(("----------Delete----------"));
		pInBuffer = (PULONG)pIrp->AssociatedIrp.SystemBuffer;
		//移除被保护进程
		RemoveProtectProcess(*pInBuffer);
		//移除被隐藏进程
		RemoveHideProcess(*pInBuffer);
		pIrp->IoStatus.Status = rtStatus;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		break;
	case IOCTL_SSDT_SEND_UP://--------------CP发送当前用户账户和密码
		KdPrint(("----------send-UP---------"));
		pUP = (PUSER_PASSWORD)pIrp->AssociatedIrp.SystemBuffer;
		//释放原始的pUL的内存和数据，并且重新分配
		ExFreePool(pc2r);
		//分配并初始化pc2r
		pc2r = (PCP_TO_R3)ExAllocatePool(NonPagedPool,sizeof(CP_TO_R3));
		RtlZeroMemory(pc2r,sizeof(CP_TO_R3));
		for (int i = 0; i < 10; i++){
			pc2r->DBNAME[i] = '#';
			pc2r->HOST[i] = '#';
		}
		//给USER赋值
		for (int i = 0; i < pUP->userLength; i++){
			if (pUP->USER[i] <= 'z' && pUP->USER[i] >= '0'){
				pc2r->USER[i] = pUP->USER[i];
			}
			else break;
		}
		pc2r->userLen = pUP->userLength;
		//给PASSWORD赋值
		for (int i = 0; i < pUP->passwordLength; i++){
			if (pUP->PASSWORD[i] <= 'z' && pUP->PASSWORD[i] >= '0'){
				pc2r->PASSWORD[i] = pUP->PASSWORD[i];
			}
			else break;
		}
		pc2r->passLen = pUP->passwordLength;
		//-------------------------插入链表----------------------------------
		userList = (PUSER_LIST)ExAllocatePool(NonPagedPool,sizeof(USER_LIST));
		RtlZeroMemory(userList,sizeof(USER_LIST));
		for (int i = 0; i < strlen(pc2r->USER); i++)
			userList->UserPassword.USER[i] = pc2r->USER[i];
		for (int i = 0; i < strlen(pc2r->PASSWORD); i++)
			userList->UserPassword.PASSWORD[i] = pc2r->PASSWORD[i];
		if (InsertUserList(userList) == STATUS_UNSUCCESSFUL)
			rtStatus = STATUS_UNSUCCESSFUL;
		userList->UserPassword.passwordLength = pUP->passwordLength;;
		userList->UserPassword.userLength = pUP->userLength;

		pIrp->IoStatus.Status = rtStatus;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		break;
	case IOCTL_SSDT_GET_UP://----------------------SkyEye获得当前用户账户，密码和DBhost,DBname
		KdPrint(("----------R3-get-UP---------"));
		pcp2r3 = (PCP_TO_R3)pIrp->AssociatedIrp.SystemBuffer;
		//设置标志为真
		SkyEyeGetReg = TRUE;
		//传USER值给pcp2r3
		for (int i = 0; i < pc2r->userLen; i++){
			if (pc2r->USER[i] <= 'z' && pc2r->USER[i] >= '0'){
				pcp2r3->USER[i] = pc2r->USER[i];
				KdPrint(("User:%c\n", pcp2r3->USER[i]));
			}
			else break;
		}
		pcp2r3->userLen = pc2r->userLen;
		//传PASSWORD值给pcp2r3
		for (int i = 0; i < strlen(pc2r->PASSWORD); i++){
			if (pc2r->PASSWORD[i] <= 'z' && pc2r->PASSWORD[i] >= '0'){
				pcp2r3->PASSWORD[i] = pc2r->PASSWORD[i];
				KdPrint(("Password:%c\n", pcp2r3->PASSWORD[i]));
			}
			else break;
		}
		pcp2r3->passLen = pc2r->passLen;
		//读取注册表DB相关值并传给pc2r
		QueryDBKey();

		for (int i = 0; i < strlen(pc2r->HOST); i++)
			pcp2r3->HOST[i] = pc2r->HOST[i];
		for (int i = 0; i < strlen(pc2r->DBNAME); i++)
			pcp2r3->DBNAME[i] = pc2r->DBNAME[i];
		//设置回原值
		SkyEyeGetReg = FALSE;
		pIrp->IoStatus.Status = rtStatus;
		pIrp->IoStatus.Information = sizeof(CP_TO_R3);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		break;
	case IOCTL_SSDT_SET_REG://----------------------SKyEye设置注册表键值
		pRTR = (PR3_TO_REG)pIrp->AssociatedIrp.SystemBuffer;
		if (NT_SUCCESS(CreateKey())){
			rtStatus = SetDBKey(pRTR->dbName, pRTR->dbHost);
			rtStatus = SetSYSKey(pRTR->sysAdmin, pRTR->sysPassword);
			if (rtStatus == STATUS_UNSUCCESSFUL)
				KdPrint(("Set Register UnSuccessful!"));
			else
				KdPrint(("Set Register Successful!"));
		}
		pIrp->IoStatus.Status = rtStatus;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		break;
	case IOCTL_SSDT_GET_REG://----------------------CP从Registry读安装信息
		pRTCC = (PREG_TO_CP)pIrp->AssociatedIrp.SystemBuffer;
		QueryDBKey();
		QuerySYSKey();
		//dbHost
		for (int i = 0; i < strlen(pRTC->dbHost); i++){
			if (pRTC->dbHost[i] <= 'z' && pRTC->dbHost[i] >= '0')
				pRTCC->dbHost[i] = pRTC->dbHost[i];
			else break;
		}
		//dbName
		for (int i = 0; i < strlen(pRTC->dbName); i++){
			if (pRTC->dbName[i] <= 'z' && pRTC->dbName[i] >= '0')
				pRTCC->dbName[i] = pRTC->dbName[i];
			else break;
		}
		//sysAdmin
		for (int i = 0; i < strlen(pRTC->sysAdmin); i++){
			if (pRTC->sysAdmin[i] <= 'z' && pRTC->sysAdmin[i] >= '0')
				pRTCC->sysAdmin[i] = pRTC->sysAdmin[i];
			else
				break;
		}
		//sysPassword
		for (int i = 0; i < strlen(pRTC->sysPassword); i++){
			if (pRTC->sysPassword[i] <= 'z' && pRTC->sysPassword[i] >= '0')
				pRTCC->sysPassword[i] = pRTC->sysPassword[i];
			else break;
		}
			
		pIrp->IoStatus.Status = rtStatus;
		pIrp->IoStatus.Information = sizeof(REG_TO_CP);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		break;
	case IOCTL_SSDT_FORBIDPROCESS_ADD://-----------------------------增加禁止进程名
		addProcesName = (PForbidProcessName)pIrp->AssociatedIrp.SystemBuffer;
		PForbidProcess insertFP = (PForbidProcess)ExAllocatePool(NonPagedPool,sizeof(ForbidProcess));
		RtlZeroMemory(insertFP,sizeof(ForbidProcess));
		KdPrint(("add ProcessName: %ws\n",addProcesName->ProcessName));
		KdPrint(("add process Name:%d\n",addProcesName->length));
		for (int i = 0; i < addProcesName->length; i++)
		{
			KdPrint(("ProcessName %c\n",addProcesName->ProcessName[i]));
			insertFP->ProcessName[i] = addProcesName->ProcessName[i];
		}
		if (InsertForbidProcessList(insertFP)){
			KdPrint(("----SSDT---Insert--ForbidProcessName--Success!\n"));
		}
		pIrp->IoStatus.Status = rtStatus;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		break;
	case IOCTL_SSDT_FORBIDPROCESS_DELETE://----------------------------删除禁止进程名
		deleteProcesName = (PForbidProcessName)pIrp->AssociatedIrp.SystemBuffer;
		KdPrint(("delete ProcessName: %ws\n", deleteProcesName->ProcessName));
		PLIST_ENTRY entry = NULL;
		if (IsListEmpty(&ForbidProcessList)){
			KdPrint(("---SSDT---ForbidProcessList is Empty!"));
		}
		else{
			for (entry = ForbidProcessList.Flink;
				entry != &ForbidProcessList;
				entry = entry->Flink){

				KdPrint(("--SSDT----Entry IN delete"));
				PForbidProcess fP = CONTAINING_RECORD(entry, ForbidProcess, listEntry);
				int count = 0;//相等的数目
				for (int i = 0; i < deleteProcesName->length; i++)
				{
					if (deleteProcesName->ProcessName[i] == fP->ProcessName[i]){
						count++;
					}else break;
				}
				KdPrint(("count : %d",count));
				KdPrint(("SSDT-----Now---ProcessName:%ws\n",fP->ProcessName));

				if (deleteProcesName->length == count){
					if (RemoveForbidProcessList(fP))
						KdPrint(("----SSDT---Delete--ForbidProcessName--Success!\n"));
					else
						KdPrint(("----SSDT---Delete--ForbidProcessName--Failed!!"));
				}
				KdPrint(("SSDT--------------------------------"));
			}
		}
		
		pIrp->IoStatus.Status = rtStatus;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		break;

	case IOCTL_SSDT_CLAER_BLACK_ILST://清空链表
		clear();
		pIrp->IoStatus.Status = rtStatus;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		break;
	default:
		break;
	}
	return rtStatus;
}

VOID SSDTDriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING strSymbolLinkName;

	DbgPrint("In SSDTDriverUnload !");

	RtlInitUnicodeString(&strSymbolLinkName, SYMBOLINK_NAME_PROCESS);
	IoDeleteSymbolicLink(&strSymbolLinkName);
	IoDeleteDevice(pDriverObject->DeviceObject);

	//解除 Hook
	UnInstallSSDTHook((ULONG)ZwQuerySystemInformation);
	UnInstallSSDTHook((ULONG)ZwTerminateProcess);
	//设置进程回调为去除状态
	PsSetCreateProcessNotifyRoutineEx(CreateProcessNotifyEx, TRUE);

	//清除链表资源
	clear();

	if (pc2r != NULL)
		ExFreePool(pc2r);
	if (pRTC != NULL)
		ExFreePool(pRTC);

	DbgPrint("Out SSDT01DriverUnload !");
}

//返回 uPID 进程在隐藏列表中的索引，如果该进程在隐藏列表中不存在，则返回 -1
ULONG ValidateProcessNeedHide(ULONG uPID)
{
	ULONG i = 0;
	if (uPID == 0)
	{
		return -1;
	}

	for (i = 0; i<g_currHideArrayLen && i<MAX_PROCESS_ARRARY_LENGTH; i++)
	{
		if (g_PIDHideArray[i] == uPID)
		{
			return i;
		}
	}
	return -1;
}

//返回 uPID 进程在保护列表中的索引，如果该进程在保护列表中不存在，则返回 -1
ULONG ValidateProcessNeedProtect(ULONG uPID)
{
	ULONG i = 0;
	if (uPID == 0)
	{
		return -1;
	}

	for (i = 0; i<g_currProtectArrayLen && i<MAX_PROCESS_ARRARY_LENGTH; i++)
	{
		if (g_PIDProtectArray[i] == uPID)
		{
			return i;
		}
	}
	return -1;
}

//在进程隐藏列表中插入新的进程 ID
ULONG InsertHideProcess(ULONG uPID)
{
	KdPrint(("Insert Hide Process====>\n"));
	if (ValidateProcessNeedHide(uPID) == -1 &&
		g_currHideArrayLen < MAX_PROCESS_ARRARY_LENGTH)
	{
		g_PIDHideArray[g_currHideArrayLen++] = uPID;
		return TRUE;
	}
	return FALSE;
}

//从进程隐藏列表中移除进程 ID
ULONG RemoveHideProcess(ULONG uPID)
{
	KdPrint(("Remove Hide Process====>\n"));
	ULONG uIndex = ValidateProcessNeedHide(uPID);
	if (uIndex != -1)
	{
		g_PIDHideArray[uIndex] = g_PIDHideArray[g_currHideArrayLen--];
		return TRUE;
	}
	return FALSE;
}

//在进程保护列表中插入新的进程 ID
ULONG InsertProtectProcess(ULONG uPID)
{
	KdPrint(("Insert Protect Process====>\n"));

	if (ValidateProcessNeedProtect(uPID) == -1 && g_currProtectArrayLen < MAX_PROCESS_ARRARY_LENGTH)
	{
		g_PIDProtectArray[g_currProtectArrayLen++] = uPID;

		return TRUE;
	}
	return FALSE;
}

//在进程保护列表中移除一个进程 ID
ULONG RemoveProtectProcess(ULONG uPID)
{
	KdPrint(("Remove Protect Process====>\n"));
	ULONG uIndex = ValidateProcessNeedProtect(uPID);
	if (uIndex != -1)
	{
		g_PIDProtectArray[uIndex] = g_PIDProtectArray[g_currProtectArrayLen--];

		return TRUE;
	}
	return FALSE;
}

//NEW的NtQuerySystemInformation
NTSTATUS MyNtQuerySystemInformation(
	__in SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
	__in ULONG SystemInformationLength,
	__out_opt PULONG ReturnLength
	)
{
	//KdPrint(("MyNtQuerySystemInformation===>\n"));
	NTSTATUS rtStatus;
	pOldNtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)oldSysServiceAddr[SYSCALL_INDEX(ZwQuerySystemInformation)];

	rtStatus = pOldNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

	if (NT_SUCCESS(rtStatus)){
		/*
			1.确定当前Query的是进程信息
			2.提取当前SystemInformation的PSYSTEM_PROCESS_INFORMATION
			3.开始循环遍历整个PSYSTEM_PROCESS_INFORMATION链
				3-1.获得当前PROCESS_INFORMATION的UniqueProcessId
				3-2.判断当前CurrProcessId是否在Hide数组内
				3-3.在Hide数组内：
					判断PrevProcessInfo是否为空，也就是说，是否是第一个ProcessInfo
					如果是：就将此进程信息从信息链中抹除
					如果不是：则确定了刚开始遍历就发现了要隐藏的ProcessInfo,抹除它
					
					其次呢，无论是不是第一个ProcessInfo都要判断是否还有下一个ProcessInfo，
					因为有不同的处理方式：1)跳过 2)当前直接置零/空
				3-4.不在Hide数组内：
					跳过当前，继续执行NextEntry
			
		*/

		if (SystemProcessInformation == SystemInformationClass)//确定当前Query的是进程信息
		{
			PSYSTEM_PROCESS_INFORMATION pPrevProcessInfo = NULL;
			PSYSTEM_PROCESS_INFORMATION pCurrProcessInfo = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;

			while (pCurrProcessInfo != NULL)//确定当前访问的进程信息不为空
			{
				//获取当前遍历的 SYSTEM_PROCESS_INFORMATION 节点的进程名称和进程 ID
				ULONG uPID = (ULONG)pCurrProcessInfo->UniqueProcessId;
				UNICODE_STRING strTmpProcessName = pCurrProcessInfo->ImageName;//可执行文件名

				//判断当前遍历的这个进程是否为需要隐藏的进程
				if (ValidateProcessNeedHide(uPID) != -1)
				{
					if (pPrevProcessInfo)
					{
						if (pCurrProcessInfo->NextEntryOffset)
						{
							//将当前这个进程(即要隐藏的进程)从 SystemInformation 中摘除(更改链表偏移指针实现)
							pPrevProcessInfo->NextEntryOffset += pCurrProcessInfo->NextEntryOffset;
						}
						else
						{
							//说明当前要隐藏的这个进程是进程链表中的最后一个
							pPrevProcessInfo->NextEntryOffset = 0;
						}
					}
					else
					{
						//第一个遍历到得进程就是需要隐藏的进程
						if (pCurrProcessInfo->NextEntryOffset)
						{
							(PCHAR)SystemInformation += pCurrProcessInfo->NextEntryOffset;
						}
						else
						{
							SystemInformation = NULL;
						}
					}
				}

				//遍历下一个 SYSTEM_PROCESS_INFORMATION 节点
				pPrevProcessInfo = pCurrProcessInfo;

				//遍历结束
				if (pCurrProcessInfo->NextEntryOffset)
				{
					pCurrProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(((PCHAR)pCurrProcessInfo) + pCurrProcessInfo->NextEntryOffset);
				}
				else
				{
					pCurrProcessInfo = NULL;
				}
			}
		}
	}

	return rtStatus;
}

//NEW的NtTerminateProcess
NTSTATUS MyNtTerminateProcess(
	__in_opt HANDLE ProcessHandle,
	__in NTSTATUS ExitStatus
	)
{
	//KdPrint(("MyNtTerminateProcess===>\n"));

	ULONG uPID;
	NTSTATUS rtStatus;
	PCHAR pStrProcName;
	PEPROCESS pEProcess;
	ANSI_STRING strProcName;

	/*
		1.通过ObReferenceObjectByHandle获得当前进程句柄的EPROCESS;
		2.导出packUp过的ZwTerminateProcess函数指针;
		3.由EPROCESS获得进程的PID;
		4.检查当前PID是否在Terminate表中：
		4-1.在的话：判断是否为系统进程，因为PsGetCurrentProcess获得的是系统进程
		若不是那就让返回无权限；
		4-2.不在的话：调用旧的ZwTerminateProcess函数
		*/

	//通过进程句柄来获得该进程所对应的 FileObject 对象，由于这里是进程对象，自然获得的是 EPROCESS 对象
	rtStatus = ObReferenceObjectByHandle(ProcessHandle, FILE_READ_DATA, NULL, KernelMode, &pEProcess, NULL);

	if (!NT_SUCCESS(rtStatus))
	{
		return rtStatus;
	}

	//保存 SSDT 中原来的 NtTerminateProcess 地址
	pOldNtTerminateProcess = (NTTERMINATEPROCESS)oldSysServiceAddr[SYSCALL_INDEX(ZwTerminateProcess)];

	//通过该函数可以获取到进程名称和进程 ID，该函数在内核中实质是导出的(在 WRK 中可以看到)
	//但是 ntddk.h 中并没有到处，所以需要自己声明才能使用

	uPID = (ULONG)PsGetProcessId(pEProcess);
	pStrProcName = (PCHAR)PsGetProcessImageFileName(pEProcess);

	KdPrint(("-----SSDT------ImageFile Name: %s\n", pStrProcName));

	//通过进程名来初始化一个 ASCII 字符串
	RtlInitAnsiString(&strProcName, pStrProcName);

	//如果要结束的进程在受保护的列表中，
	if (ValidateProcessNeedProtect(uPID) != -1)
	{
		KdPrint(("ValidateProcessNeedProtect Exist------>\n"));
		//确保调用者进程能够结束(这里主要是指 taskmgr.exe)
		if (uPID != (ULONG)PsGetProcessId(PsGetCurrentProcess()))
		{
			//如果该进程是所保护的的进程的话，则返回权限不够的异常即可
			return STATUS_ACCESS_DENIED;
		}
	}

	//对于非保护的进程可以直接调用原来 SSDT 中的 NtTerminateProcess 来结束进程
	rtStatus = pOldNtTerminateProcess(ProcessHandle, ExitStatus);

	return rtStatus;
}