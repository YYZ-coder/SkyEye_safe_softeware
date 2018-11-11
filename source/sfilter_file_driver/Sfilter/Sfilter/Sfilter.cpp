#include "stdafx.h"

ULONG intest=0;

typedef struct _SfilterExtension
{
	UNICODE_STRING symbolname;
	UNICODE_STRING devicename;
	PDEVICE_OBJECT sfliterdeviceobject; 
	

}SfilterDeviceExtension,*PSfilterDeviceExtension;
//全局变量*******************************************
PDRIVER_OBJECT gSFilterDriverObject = NULL; //保存由/O管理器生成并传入的驱动对象
PDEVICE_OBJECT gSFilterControlDeviceObject = NULL; //保存由本过滤驱动生成的控制设备对象
FAST_MUTEX gSfilterAttachLock; //定义一个快速互斥结构变量(对象)
ULONG SfDebug =1;
LIST_ENTRY listentry;      //表头
ULONG MYDEBUG=0x0010;
HASH_TABLE hashtable; 
HASH_TABLE hashtable_result;
HASH_TABLE_CAREFILE hashtable_carefile;

UserInfo user_info;
LIST_ENTRY operationrecord_listentry;   //操作信息链表
NPAGED_LOOKASIDE_LIST looksidelist;    //这个结构是为了避免频繁分配内存导致的内存碎片过多
NPAGED_LOOKASIDE_LIST looksidelist_result;
NPAGED_LOOKASIDE_LIST  looksidelist_carefile;
NPAGED_LOOKASIDE_LIST carefile_verify;

extern BOOLEAN CareFile_Initilized=FALSE;   //记录是否初始化成功了关心文件链表

size_t process_name_offset;  //这个变量保存了PEPRCOESS结构里面的进程名字的偏移量，每个windows系统的这个结构都有区别，所以在确定的
            
//的DriverEntry这个函数会在system进程运行来计算出这个偏移量

KSPIN_LOCK test_lock; 
POperationResult resultfortest;
KSPIN_LOCK myevents_lock;

PThreadEvents myevents[HASH_TABLE_LENGTH]; //存储从应用程序发来的事件，在满足条件的时候设置事件用来通知事件


#if WINVER >= 0x0501
 SF_DYNAMIC_FUNCTION_POINTERS gSfDynamicFunctions = {0}; //定义该结构变量并初始化为0
 ULONG gSfOsMajorVersion = 0;        //操作系统主版本号
ULONG gSfOsMinorVersion = 0;        //操作系统副版本号
#endif
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)

#if DBG && WINVER >= 0x0501
#pragma alloc_text(PAGE, DriverUnload)
#endif

#pragma alloc_text(INIT, SfReadDriverParameters)
#pragma alloc_text(INIT, ProcessOffsetInit)
#pragma alloc_text(PAGE, SfFsNotification)
#pragma alloc_text(PAGE, SfCreate)
#pragma alloc_text(PAGE, SfCleanupClose)
#pragma alloc_text(PAGE, SfFsControl)
#pragma alloc_text(PAGE, SfFsControlMountVolume)
#pragma alloc_text(PAGE, SfFsControlMountVolumeComplete)
#pragma alloc_text(PAGE, SfFsControlLoadFileSystem)
#pragma alloc_text(PAGE, SfFsControlLoadFileSystemComplete)
#pragma alloc_text(PAGE, SfFastIoCheckIfPossible)
#pragma alloc_text(PAGE, SfFastIoRead)
#pragma alloc_text(PAGE, SfFastIoWrite)
#pragma alloc_text(PAGE, SfFastIoQueryBasicInfo)
#pragma alloc_text(PAGE, SfFastIoQueryStandardInfo)
#pragma alloc_text(PAGE, SfFastIoLock)
#pragma alloc_text(PAGE, SfFastIoUnlockSingle)
#pragma alloc_text(PAGE, SfFastIoUnlockAll)
#pragma alloc_text(PAGE, SfFastIoUnlockAllByKey)
#pragma alloc_text(PAGE, SfFastIoDeviceControl)
#pragma alloc_text(PAGE, SfFastIoDetachDevice)
#pragma alloc_text(PAGE, SfFastIoQueryNetworkOpenInfo)
#pragma alloc_text(PAGE, SfFastIoMdlRead)
#pragma alloc_text(PAGE, SfFastIoPrepareMdlWrite)
#pragma alloc_text(PAGE, SfFastIoMdlWriteComplete)
#pragma alloc_text(PAGE, SfFastIoReadCompressed)
#pragma alloc_text(PAGE, SfFastIoWriteCompressed)
#pragma alloc_text(PAGE, SfFastIoQueryOpen)
#pragma alloc_text(PAGE, SfAttachDeviceToDeviceStack)
#pragma alloc_text(PAGE, SfAttachToFileSystemDevice)
#pragma alloc_text(PAGE, SfDetachFromFileSystemDevice)
#pragma alloc_text(PAGE, SfAttachToMountedDevice)
#pragma alloc_text(PAGE, SfIsAttachedToDevice)
#pragma alloc_text(PAGE, SfIsAttachedToDeviceW2K)
#pragma alloc_text(PAGE, SfIsShadowCopyVolume)

#if WINVER >= 0x0501
#pragma alloc_text(INIT, SfLoadDynamicFunctions)
#pragma alloc_text(INIT, SfGetCurrentVersion)
#pragma alloc_text(PAGE, SfEnumerateFileSystemVolumes)
#pragma alloc_text(PAGE, SfIsAttachedToDeviceWXPAndLater)
#endif

#endif

bool MY_DEBUG(ULONG a,ULONG b) {if(a&&b==0x0000)return false;else return true;}
//************************************关于初始化数据的函数**********************
VOID ProcessOffsetInit()
 {
	 ULONG i;
	 PEPROCESS current;
	 current=::PsGetCurrentProcess();

	 for(i=0;i<3*4*1024;i++)
	 {
		 if(!strncmp("System",(PCHAR)current+i,strlen("System\n")))
		 {
			 process_name_offset=i;
			 KdPrint(("找到system字符，初始化完毕：%d %0x",i,i));
		     break;
		 }
	 }
}
VOID InitMyEvents()
{
	KdPrint(("开始初始化全局事件句柄数组!\n\n"));
	for(int i=0;i<HASH_TABLE_LENGTH;i++)
	{
		myevents[i]=(PThreadEvents)::ExAllocatePool(NonPagedPool,sizeof(ThreadEvents));
		myevents[i]->OperationType=-1;
		myevents[i]->event=NULL;
		myevents[i]->waitstatue=false;
		::KeInitializeSpinLock(&myevents[i]->nodelock);
		myevents[i]->oldirql=0;

	}
	KdPrint(("初始化完毕！\n\n"));
}
VOID InitHashTable()
{
        PHASH_TABLE a=&hashtable;
		PHASH_TABLE b=&hashtable_result;

		
		//a=(PHASH_TABLE)ExAllocatePoolWithTag(NonPagedPool,sizeof(HASH_TABLE),MEM_HASH_TAG);	
	
		a->Length=HASH_TABLE_LENGTH;
		b->Length=HASH_TABLE_LENGTH;
		                    
		for(int i=0;i<a->Length;i++)
		{
			PHASH_TABLE_NODE m=&(a->link[i]);
			m->number=0;
			m->OperationType=-1;     //初始化操作类型为-1（即为空）
			::InitializeListHead(&(m->entry));
			::KeInitializeSpinLock(&(m->node_lock));
			m->initilzed=true;
			KdPrint(("the %d node is init:%d\n",i,m->initilzed?1:0));
		}

		KdPrint(("初始化HASH_TABLE成功,正在初始化HASH_TABLE RESULT.....\n\n"));
		//KdPrint(("attmpt to access hashtable:%d ,inilized:%d",hashtable.link[10].number,hashtable.link[10].initilzed?1:0));
	   		for(int i=0;i<b->Length;i++)
		{
			PHASH_TABLE_NODE m=&(b->link[i]);
			m->number=0;
			m->OperationType=-1;     //初始化操作类型为-1（即为空）
			::InitializeListHead(&(m->entry));
			::KeInitializeSpinLock(&(m->node_lock));
			m->initilzed=true;
			m->totalnumbers=0;
			m->NodeID=i;
			
			KdPrint(("the %d node is init:%d",i,m->initilzed?1:0));
		}
        KdPrint(("初始化HASH_TABLE RESULT成功!\n\n"));
}
VOID InitUserInfo()
{
	user_info.Security_Level=ROLE_SYSTEM;
	user_info.UserID=2;
}
#ifdef __cplusplus
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING  RegistryPath);
#endif

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath )
{
	KdPrint(("2.53\n"));
if(!FlagOn(SfDebug,0x0000020))
	KdPrint(("true\n"));
UNICODE_STRING symbolname;
//KdPrint(("sfilter in!\n"));
PFAST_IO_DISPATCH fastIoDispatch; //定义FAST_IO_DISPATCH结构变量
UNICODE_STRING nameString; //定义名字串结构变量
NTSTATUS status; //状态码
ULONG i;

::ProcessOffsetInit();
::KeInitializeSpinLock(&test_lock);
::InitHashTable();
InitMyEvents();
::InitUserInfo();
_try{
::ExInitializeNPagedLookasideList(&looksidelist
	,NULL,
	NULL,
	0,
	sizeof(OperationInformation),
	LOOKASIDE_LIST_FOR_OPERATION,
	0);
::ExInitializeNPagedLookasideList(&looksidelist_result
	,NULL,
	NULL,
	0,
	sizeof(OperationResult),
	LOOKASIDE_LIST_FOR_RESULT,
	0);
::ExInitializeNPagedLookasideList(&looksidelist_carefile
	,NULL,
	NULL,
	0,
	sizeof(CareFile),
	LOOKASIDE_LIST_FOR_CAREFILE,
	0);
::ExInitializeNPagedLookasideList(&carefile_verify
	,NULL,
	NULL,
	0,
	sizeof(CareFile),
	LOOKASIDE_LIST_FOR_CAREFILE_VERIFY,
	0);
}
_except(STATUS_ACCESS_VIOLATION)
{
	KdPrint(("初始化LookAsidelist失败！\n"));
}

::InitHashTable_CareFile();	    //必须要在looksid_carefile成功初始化之后才能开始初始化

//::InitializeListHead(&listentry);  //初始化链表
//KdPrint(("初始化链表,哈希表完毕1!\r\n"));

#if WINVER >= 0x0501 //条件编译,如果OS版本是WinXP以上,编译这两句,否则不编译
SfLoadDynamicFunctions();
SfGetCurrentVersion();
KdPrint(("the current system version upon Windows XP\n"));
#endif




SfReadDriverParameters( RegistryPath );
gSFilterDriverObject = DriverObject; //将I/O管理器传入的驱动对象保存到全局变量gSFilterDriverObject中


#if DBG && WINVER >= 0x0501 //若OS版本是xp以上且生成checked版,编译这些语句,否则不编译
if (NULL != gSfDynamicFunctions.EnumerateDeviceObjectList)
{
gSFilterDriverObject->DriverUnload = DriverUnload; //注册驱动卸载函数
}
#endif

ExInitializeFastMutex( &gSfilterAttachLock ); //初始化"FastMutex(快速互斥)"对象,以后多线程只能互斥访问它



//创建控制设备名称
RtlInitUnicodeString( &nameString, L"\\FileSystem\\Filters\\SFilter" ); //用来创建文件系统控制设备对象

//创建控制设备对象
status = IoCreateDevice( DriverObject,
sizeof(SfilterDeviceExtension), //没有 设备扩展
&nameString, //设备名: FileSystem\\Filters\\SFilter
FILE_DEVICE_DISK_FILE_SYSTEM, //设备类型: 磁盘文件系统
FILE_DEVICE_SECURE_OPEN, //设备特征: 对发送到CDO的打开请求进行安全检查
FALSE, //生成一个在用户模式下使用的设备
&gSFilterControlDeviceObject ); //接收生成的"控制设备对象"

if (status == STATUS_OBJECT_PATH_NOT_FOUND) //判断是否 未找到路径
{
RtlInitUnicodeString( &nameString, L"\\FileSystem\\SFilterCDO" ); //重新创建 控制设备名称
status = IoCreateDevice( DriverObject, 0,
&nameString, //设备名: FileSystem\\SFilterCDO
FILE_DEVICE_DISK_FILE_SYSTEM,
FILE_DEVICE_SECURE_OPEN,
FALSE,
&gSFilterControlDeviceObject ); //接收生成的 控制设备对象
}


if (!NT_SUCCESS( status )) //判断IoCreateDevice调用是否成功
{
KdPrint(( "SFilter!DriverEntry: Error creating control device object \"%wZ\", status=%08x\n", &nameString, status ));
return status; //错误返回(创建CDO失败)
}

//创建链接用于应用程序打开
	::RtlInitUnicodeString(&symbolname,L"\\DosDevices\\SFilterDevice");
	status=::IoCreateSymbolicLink(&symbolname,&nameString);
	if(!NT_SUCCESS(status))
	{
		::IoDeleteSymbolicLink(&symbolname);
		status=::IoCreateSymbolicLink(&symbolname,&nameString);
		if(!NT_SUCCESS(status))
		{
			::IoDeleteSymbolicLink(&symbolname);
			KdPrint(("create symbollink name failed!\n"));
			return status;
		}
	
	}
	KdPrint(("create symbol link successfully!\n"));
	PSfilterDeviceExtension deviceextension=(PSfilterDeviceExtension)DriverObject->DriverExtension;
	deviceextension->devicename=nameString;
	deviceextension->symbolname=symbolname;
	deviceextension->sfliterdeviceobject=gSFilterControlDeviceObject;

//注册默认派遣函数
for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
{
DriverObject->MajorFunction[i] = SfPassThrough;
}
//注册具体派遣函数
DriverObject->MajorFunction[IRP_MJ_CREATE] = SfCreate;
DriverObject->MajorFunction[IRP_MJ_CREATE_NAMED_PIPE] = SfCreate;
DriverObject->MajorFunction[IRP_MJ_CREATE_MAILSLOT] = SfCreate;
DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = SfFsControl;
DriverObject->MajorFunction[IRP_MJ_CLEANUP] = SfCleanupClose;
DriverObject->MajorFunction[IRP_MJ_CLOSE] = SfCleanupClose;
DriverObject->MajorFunction[IRP_MJ_READ]=SfRead;
DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]=SfDirectoryControl;
DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]=SfSetInformation;
DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]=SfDeviceControl;

fastIoDispatch = (PFAST_IO_DISPATCH)ExAllocatePoolWithTag( NonPagedPool, //从非分页池中分配
sizeof( FAST_IO_DISPATCH ), //要分配的字节数
SFLT_POOL_TAG); //指定一个4字节的标签(前面已宏定义:'tlFS')

if (!fastIoDispatch) //内存分配失败
{
IoDeleteDevice( gSFilterControlDeviceObject ); //删除上面创建的CDO
return STATUS_INSUFFICIENT_RESOURCES; //返回一个错误status码(资源不足)
}
RtlZeroMemory( fastIoDispatch, sizeof( FAST_IO_DISPATCH ) );
DriverObject->FastIoDispatch = fastIoDispatch; //将FastIo分派表保存到驱动对象的FastIoDispatch域
fastIoDispatch->SizeOfFastIoDispatch = sizeof( FAST_IO_DISPATCH ); //设置FastIo分派表的长度域

fastIoDispatch->FastIoCheckIfPossible = SfFastIoCheckIfPossible; //设置FastIo分派函数,共21个
fastIoDispatch->FastIoRead = SfFastIoRead;
fastIoDispatch->FastIoWrite = SfFastIoWrite;
fastIoDispatch->FastIoQueryBasicInfo = SfFastIoQueryBasicInfo;
fastIoDispatch->FastIoQueryStandardInfo = SfFastIoQueryStandardInfo;
fastIoDispatch->FastIoLock = SfFastIoLock;
fastIoDispatch->FastIoUnlockSingle = SfFastIoUnlockSingle;
fastIoDispatch->FastIoUnlockAll = SfFastIoUnlockAll;
fastIoDispatch->FastIoUnlockAllByKey = SfFastIoUnlockAllByKey;
fastIoDispatch->FastIoDeviceControl = SfFastIoDeviceControl;
fastIoDispatch->FastIoDetachDevice = SfFastIoDetachDevice;
fastIoDispatch->FastIoQueryNetworkOpenInfo = SfFastIoQueryNetworkOpenInfo;
fastIoDispatch->MdlRead = SfFastIoMdlRead;
fastIoDispatch->MdlReadComplete = SfFastIoMdlReadComplete;
fastIoDispatch->PrepareMdlWrite = SfFastIoPrepareMdlWrite;
fastIoDispatch->MdlWriteComplete = SfFastIoMdlWriteComplete;
fastIoDispatch->FastIoReadCompressed = SfFastIoReadCompressed;
fastIoDispatch->FastIoWriteCompressed = SfFastIoWriteCompressed;
fastIoDispatch->MdlReadCompleteCompressed = SfFastIoMdlReadCompleteCompressed;
fastIoDispatch->MdlWriteCompleteCompressed = SfFastIoMdlWriteCompleteCompressed;
fastIoDispatch->FastIoQueryOpen = SfFastIoQueryOpen;
                          


//--------------------------------注册fsFilter回调函数-------------------------------

#if WINVER >= 0x0501 //如果OS版本为WinXP以上,编译这段代码,否则不编译
{
FS_FILTER_CALLBACKS fsFilterCallbacks;
if (NULL != gSfDynamicFunctions.RegisterFileSystemFilterCallbacks)
{
fsFilterCallbacks.SizeOfFsFilterCallbacks = sizeof( FS_FILTER_CALLBACKS );
fsFilterCallbacks.PreAcquireForSectionSynchronization = SfPreFsFilterPassThrough;
fsFilterCallbacks.PostAcquireForSectionSynchronization = SfPostFsFilterPassThrough;
fsFilterCallbacks.PreReleaseForSectionSynchronization = SfPreFsFilterPassThrough;
fsFilterCallbacks.PostReleaseForSectionSynchronization = SfPostFsFilterPassThrough;
fsFilterCallbacks.PreAcquireForCcFlush = SfPreFsFilterPassThrough;
fsFilterCallbacks.PostAcquireForCcFlush = SfPostFsFilterPassThrough;
fsFilterCallbacks.PreReleaseForCcFlush = SfPreFsFilterPassThrough;
fsFilterCallbacks.PostReleaseForCcFlush = SfPostFsFilterPassThrough;
fsFilterCallbacks.PreAcquireForModifiedPageWriter = SfPreFsFilterPassThrough;
fsFilterCallbacks.PostAcquireForModifiedPageWriter = SfPostFsFilterPassThrough;
fsFilterCallbacks.PreReleaseForModifiedPageWriter = SfPreFsFilterPassThrough;
fsFilterCallbacks.PostReleaseForModifiedPageWriter = SfPostFsFilterPassThrough;

status = (gSfDynamicFunctions.RegisterFileSystemFilterCallbacks)( DriverObject, &fsFilterCallbacks );
if (!NT_SUCCESS( status ))
{
DriverObject->FastIoDispatch = NULL;
ExFreePool( fastIoDispatch );
IoDeleteDevice( gSFilterControlDeviceObject );
return status;
}
}
}
#endif


status = IoRegisterFsRegistrationChange( DriverObject, SfFsNotification );//注册文件系统改变时触发的回调函数，在完成函数做一些事情，然后在后面绑定动态绑定新文件系统的CDO
if (!NT_SUCCESS( status ))
{
KdPrint(( "SFilter!DriverEntry: Error registering FS change notification, status=%08x\n", status ));

DriverObject->FastIoDispatch = NULL; //注销指向fastIo函数组的指针为NULL
ExFreePoolWithTag( fastIoDispatch, SFLT_POOL_TAG ); //释放分配给fastIo函数组的内存
IoDeleteDevice( gSFilterControlDeviceObject ); //删除上面创建的CDO
return status; //错误返回
}

{

PDEVICE_OBJECT rawDeviceObject;
PFILE_OBJECT fileObject;
RtlInitUnicodeString( &nameString, L"\\Device\\RawDisk" );//普通的磁盘


/*
IoGetDeviceObjectPointer函数的功能是:
它从下层的设备对象名称来获得下层设备指针。该函数造成了对下层设备对象以及下层设备对象所对应的文件对象的引用。
如果本层驱动在卸载之前对下层的设备对象的引用还没有消除，则下层驱动的卸载会被停止。因此必须要消除对下层设备对象的引用。
但是程序一般不会直接对下层设备对象的引用减少。因此只要减少对文件对象的引用就可以减少文件对象和设备对象两个对象的引用。
事实上，IoGetDeviceObjectPointer返回的并不是下层设备对象的指针，而是该设备堆栈中顶层的设备对象的指针。


IoGetDeviceObjectPointer函数的调用必须在 IRQL=PASSIVE_LEVEL的级别上运行。
*/

status = IoGetDeviceObjectPointer( &nameString, FILE_READ_ATTRIBUTES, &fileObject, &rawDeviceObject );//
if (NT_SUCCESS( status ))
{
SfFsNotification( rawDeviceObject, TRUE );   //创建一个过滤设备在FSCDO上
ObDereferenceObject( fileObject ); //这里减少对文件对象的引用
}

RtlInitUnicodeString( &nameString, L"\\Device\\RawCdRom" );   //找到Cd设备（光驱）
status = IoGetDeviceObjectPointer( &nameString, FILE_READ_ATTRIBUTES, &fileObject, &rawDeviceObject );
if (NT_SUCCESS( status ))
{
SfFsNotification( rawDeviceObject, TRUE );   
ObDereferenceObject( fileObject );//这里减少对文件对象的引用
}
}

ClearFlag( gSFilterControlDeviceObject->Flags, DO_DEVICE_INITIALIZING );
return STATUS_SUCCESS;
}


// 驱动卸载函数
#if DBG && WINVER >= 0x0501
VOID DriverUnload( IN PDRIVER_OBJECT DriverObject )
{
//KdPrint(("DriverUnload : start display the hashtable\n\n"));

PSFILTER_DEVICE_EXTENSION devExt;
PFAST_IO_DISPATCH fastIoDispatch;
NTSTATUS status;
ULONG numDevices;
ULONG i;
LARGE_INTEGER interval;

PDEVICE_OBJECT devList[DEVOBJ_LIST_SIZE];
ASSERT(DriverObject == gSFilterDriverObject);
SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES, ("SFilter!DriverUnload: Unloading driver (%p)\n", DriverObject) );

for(int i=0;i<hashtable.Length;i++)
{
     while(!IsListEmpty(&hashtable.link[i].entry))
		{
		PLIST_ENTRY entry=RemoveHeadList(&hashtable.link[i].entry);;
		POperationInformation m=CONTAINING_RECORD(entry,OperationInformation,listentry);
		::ExFreeToNPagedLookasideList(&looksidelist,m);
		}	
	//::ExFreeToNPagedLookasideList(&looksidelist,hashtable.link[i].entry);
}
::ExFreePool(&hashtable);
::ExDeleteNPagedLookasideList(&looksidelist);

IoUnregisterFsRegistrationChange( DriverObject, SfFsNotification );
for (;;)
{
ASSERT( NULL != gSfDynamicFunctions.EnumerateDeviceObjectList );
status = (gSfDynamicFunctions.EnumerateDeviceObjectList)( DriverObject, devList, sizeof(devList), &numDevices);
if (numDevices <= 0)
{
break;
}

numDevices = min( numDevices, DEVOBJ_LIST_SIZE );
for (i=0; i < numDevices; i++)
{
devExt =(PSFILTER_DEVICE_EXTENSION) devList[i]->DeviceExtension;
if (NULL != devExt)
{
IoDetachDevice( devExt->AttachedToDeviceObject );
}
}

interval.QuadPart = (5 * DELAY_ONE_SECOND); //delay 5 seconds
KeDelayExecutionThread( KernelMode, FALSE, &interval );
for (i=0; i < numDevices; i++)
{
if (NULL != devList[i]->DeviceExtension)
{
SfCleanupMountedDevice( devList[i] );
}
else
{
ASSERT(devList[i] == gSFilterControlDeviceObject);
gSFilterControlDeviceObject = NULL;
}

IoDeleteDevice( devList[i] );
ObDereferenceObject( devList[i] );
}
}

fastIoDispatch = DriverObject->FastIoDispatch;
DriverObject->FastIoDispatch = NULL;
ExFreePool( fastIoDispatch );
}
#endif



// SfLoadDynamicFunctions函数(仅在WindowsXP系统下编译该函数)
#if WINVER >= 0x0501
VOID SfLoadDynamicFunctions()
{

/*
动态加载下面的这些内核函数。使用动态加载所谓理由是：在低版本的Windows操作系统上，这段程序依然可以加载成功。
*/

UNICODE_STRING functionName;
RtlZeroMemory( &gSfDynamicFunctions, sizeof( gSfDynamicFunctions ) ); //将gSfDynamicFunctions结构变量清0

RtlInitUnicodeString( &functionName, L"FsRtlRegisterFileSystemFilterCallbacks" );
gSfDynamicFunctions.RegisterFileSystemFilterCallbacks =(PSF_REGISTER_FILE_SYSTEM_FILTER_CALLBACKS) MmGetSystemRoutineAddress( &functionName );

RtlInitUnicodeString( &functionName, L"IoAttachDeviceToDeviceStackSafe" );
gSfDynamicFunctions.AttachDeviceToDeviceStackSafe =(PSF_ATTACH_DEVICE_TO_DEVICE_STACK_SAFE) MmGetSystemRoutineAddress( &functionName );

RtlInitUnicodeString( &functionName, L"IoEnumerateDeviceObjectList" );
gSfDynamicFunctions.EnumerateDeviceObjectList =(PSF_ENUMERATE_DEVICE_OBJECT_LIST) MmGetSystemRoutineAddress( &functionName );

RtlInitUnicodeString( &functionName, L"IoGetLowerDeviceObject" );
gSfDynamicFunctions.GetLowerDeviceObject =(PSF_GET_LOWER_DEVICE_OBJECT)MmGetSystemRoutineAddress( &functionName );

RtlInitUnicodeString( &functionName, L"IoGetDeviceAttachmentBaseRef" );
gSfDynamicFunctions.GetDeviceAttachmentBaseRef =(PSF_GET_DEVICE_ATTACHMENT_BASE_REF)MmGetSystemRoutineAddress( &functionName );

RtlInitUnicodeString( &functionName, L"IoGetDiskDeviceObject" );
gSfDynamicFunctions.GetDiskDeviceObject =(PSF_GET_DISK_DEVICE_OBJECT)MmGetSystemRoutineAddress( &functionName );

RtlInitUnicodeString( &functionName, L"IoGetAttachedDeviceReference" );
gSfDynamicFunctions.GetAttachedDeviceReference =(PSF_GET_ATTACHED_DEVICE_REFERENCE) MmGetSystemRoutineAddress( &functionName );

RtlInitUnicodeString( &functionName, L"RtlGetVersion" );
gSfDynamicFunctions.GetVersion =(PSF_GET_VERSION) MmGetSystemRoutineAddress( &functionName );
}
#endif



//SfGetCurrentVersion函数(仅WinXP下编译该函数)
#if WINVER >= 0x0501
VOID SfGetCurrentVersion()
{
if (NULL != gSfDynamicFunctions.GetVersion)
{
RTL_OSVERSIONINFOW versionInfo;
NTSTATUS status;

versionInfo.dwOSVersionInfoSize = sizeof( RTL_OSVERSIONINFOW );
status = (gSfDynamicFunctions.GetVersion)( &versionInfo );
ASSERT( NT_SUCCESS( status ) );
gSfOsMajorVersion = versionInfo.dwMajorVersion;
gSfOsMinorVersion = versionInfo.dwMinorVersion;
}
else
{
PsGetVersion( &gSfOsMajorVersion, &gSfOsMinorVersion, NULL, NULL );
}
}
#endif




VOID SfFsNotification( IN PDEVICE_OBJECT DeviceObject, IN BOOLEAN FsActive )
{

/*
SfFsNotification函数：
它创建一个设备对象，并将它附加到指定的文件系统控制设备对象(File System CDO)的对象栈上。这就允许这个设备对象过滤所有发送给文件系统的请求。
这样，我们就能够获得一个挂载卷的请求，就可以附加到这个新的卷设备对象的设备对象栈上。

当SfFsNotification函数调用完毕以后，我们的过滤驱动设备对象就能够接收到发送到文件系统CDO的请求，即接收到IRP_MJ_FILE_SYSTEM_CONTROL，或者说，
文件系统控制设备已经被绑定，可以动态监控卷的挂载了。那么以后的工作就是要完成对卷的监控绑定了。


参数说明:

DeviceObject: 它指向文件系统的控制设备对象(CDO)。即 被激活或则撤消的File System CDO
FsActive: 值为TRUE，表示文件系统的激活。值为FALSE，表示文件系统的卸载。

*/

UNICODE_STRING name; //定义结构变量
WCHAR nameBuffer[MAX_DEVNAME_LENGTH]; //定义宽字符缓冲区,长度64

PAGED_CODE();

RtlInitEmptyUnicodeString( &name, nameBuffer, sizeof(nameBuffer) ); //初始化name(成员Buffer->nameBuffer,Length=0,MaximumLength=64)
SfGetObjectName( DeviceObject, &name );
KdPrint(("SfNotification in!\n"));
SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfFsNotification: %s %p \"%wZ\" (%s)\n",
(FsActive) ? "Activating file system " : "Deactivating file system",
DeviceObject,
&name,
GET_DEVICE_TYPE_NAME(DeviceObject->DeviceType)) );

if (FsActive)
{
SfAttachToFileSystemDevice( DeviceObject, &name ); //用于完成对文件系统控制设备的绑定
}
else
{
SfDetachFromFileSystemDevice( DeviceObject );
}
}



NTSTATUS SfPassThrough( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
IoSkipCurrentIrpStackLocation( Irp );
return IoCallDriver( ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}



VOID SfDisplayCreateFileName( IN PIRP Irp )
{
PIO_STACK_LOCATION irpSp;
PUNICODE_STRING name;
GET_NAME_CONTROL nameControl;

irpSp = IoGetCurrentIrpStackLocation( Irp );
name = SfGetFileName( irpSp->FileObject, Irp->IoStatus.Status, &nameControl );
//SfDebug=SFDEBUG_DISPLAY_CREATE_NAMES;
if (irpSp->Parameters.Create.Options & FILE_OPEN_BY_FILE_ID)
{
SF_LOG_PRINT( SFDEBUG_DISPLAY_CREATE_NAMES,
("SFilter!SfDisplayCreateFileName: Opened %08x:%08x %wZ ,%wZ(FID) \n",
Irp->IoStatus.Status,
Irp->IoStatus.Information,
name));
}
else
{
SF_LOG_PRINT( SFDEBUG_DISPLAY_CREATE_NAMES,
("SFilter!SfDisplayCreateFileName: Opened %08x:%08x %wZ\n",
Irp->IoStatus.Status,
Irp->IoStatus.Information,
name));
}
//SfDebug=1;
SfGetFileNameCleanup( &nameControl );
}



NTSTATUS SfCreate( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
NTSTATUS status;

/*
在sFilter的代码中，下面的这个if语句体这样写的后果是:我们用CreateFile函数在Ring3下打开此控制设备符号链接的时候会失败

if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject))
{
Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
Irp->IoStatus.Information = 0;
IoCompleteRequest( Irp, IO_NO_INCREMENT );
return STATUS_INVALID_DEVICE_REQUEST;
}
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));*/
//KdPrint(("sfCreate 1\n"));
/*
因此，需要把上面的这个if语句体修改成如下形式:
*/
if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject))
{
KdPrint(("Sfiter:My Control Device!\n"));
Irp->IoStatus.Status = STATUS_SUCCESS;
Irp->IoStatus.Information = FILE_OPENED;
IoCompleteRequest( Irp, IO_NO_INCREMENT );
return STATUS_SUCCESS;
}


PIO_STACK_LOCATION irpsp=(PIO_STACK_LOCATION)IoGetCurrentIrpStackLocation(Irp);
PFILE_OBJECT fileobject=irpsp->FileObject;
UNICODE_STRING filename={0};
UNICODE_STRING filterfilename={0};
WCHAR *filter=L"\\Device\\HarddiskVolume1\\Windows\\fortest.txt";
WCHAR filenamebuffer[512];
ULONG length;
/*
filename.MaximumLength=1024;
filterfilename.MaximumLength=1024;
*/
do
{
	::RtlInitUnicodeString(&filterfilename,filter);
	::RtlInitEmptyUnicodeString(&filename,filenamebuffer,sizeof(WCHAR)*512);
/*filename.Buffer=(PWSTR)::ExAllocatePool(NonPagedPool,1024);

if(filename.Buffer==NULL)
{
	KdPrint(("filename allocate memory failed\n"));
	break;
}

filterfilename.Buffer=(PWSTR)::ExAllocatePool(NonPagedPool,1024);
if( filterfilename.Buffer==NULL)
{
	ExFreePool(filename.Buffer);
	break;
}

RtlZeroMemory(filename.Buffer,1024);
filename.Length=2*wcslen(string);
ASSERT(filename.MaximumLength>=filename.Length);
::RtlCopyMemory(filename.Buffer,string,filename.Length);


filterfilename.Length=2*wcslen(filter);
ASSERT(filterfilename.MaximumLength>=filterfilename.Length);
::RtlCopyMemory(filterfilename.Buffer,filter,filterfilename.Length);
//CURRENT_IRQL_PRINT();
*/
if((KeGetCurrentIrql()==PASSIVE_LEVEL))
{

length=MyFileFullPathQuery(fileobject,&filename);//获得文件名

if(::IsCreateNewFile(Irp))
{
	
}	

}
else
{
	KdPrint(("中断级别过高\n"));
}

}while(0);



SfDebug=SFDEBUG_GET_CREATE_NAMES;
if (!FlagOn( SfDebug, SFDEBUG_DO_CREATE_COMPLETION | SFDEBUG_GET_CREATE_NAMES| SFDEBUG_DISPLAY_CREATE_NAMES ))//改动过,原为!
{
	//KdPrint(("sfCreate 2\n"));
IoSkipCurrentIrpStackLocation( Irp );
return IoCallDriver( ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}
else
{
//初始化事件对象，设置完成例程。

//	KdPrint(("sfCreate 3\n"));
/*PIO_STACK_LOCATION irpSp;
PUNICODE_STRING name;
UNICODE_STRING nameto;
GET_NAME_CONTROL nameControl;
WCHAR buff[1024]=L"start";
WCHAR *filter=L"\\Device\\HarddiskVolume1\\fortest";
irpSp = IoGetCurrentIrpStackLocation( Irp );
if((KeGetCurrentIrql()==PASSIVE_LEVEL))
{
//KdPrint(("compare in2!\n"));
//name = SfGetFileName( irpSp->FileObject, Irp->IoStatus.Status, &nameControl );
name=::MyFileFullPathQueryW(irpSp->FileObject,1024);
::RtlInitUnicodeString(&nameto,filter);
if(::RtlCompareUnicodeString(name,&nameto,true)==0)
{
	 KdPrint(("该目录下的文件不允许打开:%wZ",name));
	 ::ExFreePool(name->Buffer);
     status =STATUS_SUCCESS;
     IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;

}
*/
KEVENT waitEvent;
KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
IoCopyCurrentIrpStackLocationToNext( Irp );
IoSetCompletionRoutine( Irp,
SfCreateCompletion,
&waitEvent,
TRUE,
TRUE,
TRUE );
status = IoCallDriver( ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
if (STATUS_PENDING == status)//若状态是 挂起
{
	KdPrint(("挂起!\n\n"));
NTSTATUS localStatus = KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
ASSERT(STATUS_SUCCESS == localStatus);
}

ASSERT(KeReadStateEvent(&waitEvent) || !NT_SUCCESS(Irp->IoStatus.Status));


/*if (irpSp->Parameters.Create.Options & FILE_OPEN_BY_FILE_ID)
{
::RtlInitUnicodeString(&nameto,filter);
if(::RtlCompareUnicodeString(name,&nameto,true)==0)
{
	 KdPrint(("该目录下的文件不允许打开:%wZ",&nameto));
}
else
{
	//KdPrint(("name:%wZ ,nameto: %wZ",name,&nameto));
	;
}
}
else
{
	//KdPrint(("Non FID:%wZ",name));
}
::SfGetFileNameCleanup(&nameControl);
}*/

if (FlagOn(SfDebug, (SFDEBUG_GET_CREATE_NAMES|SFDEBUG_DISPLAY_CREATE_NAMES)))
{
//SfDisplayCreateFileName( Irp );
}
status = Irp->IoStatus.Status;
IoCompleteRequest( Irp, IO_NO_INCREMENT );
return status;
}

}



NTSTATUS SfCreateCompletion( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context )
{
PKEVENT event =(PKEVENT) Context;
KSPIN_LOCK spinlock;
KIRQL irql=::KeGetCurrentIrql();//起初这里没有初始化，导致默认为是0，而在后面的kerealsespinlock里面产生了dispatch到0的错误，出发了irql_gt_zero_at_system_service的错误
KeInitializeSpinLock(&spinlock);
UNREFERENCED_PARAMETER( DeviceObject );
//UNREFERENCED_PARAMETER( Irp );

PIO_STACK_LOCATION irpsp=(PIO_STACK_LOCATION)IoGetCurrentIrpStackLocation(Irp);
PFILE_OBJECT fileobject=irpsp->FileObject;
//WCHAR buff[FILE_NAME_LENGTH_MAX];
//GET_NAME_CONTROL nameControl;
if(NT_SUCCESS(Irp->IoStatus.Status))
{
	if(fileobject!=NULL&&(irpsp->Parameters.Create.Options&FILE_DIRECTORY_FILE)!=0)
	{
	//	PUNICODE_STRING filenames=SfGetFileName(fileobject,Irp->IoStatus.Status,&nameControl);

		//MyAddToHashTable(fileobject,&spinlock,filenames);

	//	::SfGetFileNameCleanup(&nameControl);
	}
}
KeReleaseSpinLock(&spinlock,irql);
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

KeSetEvent(event, IO_NO_INCREMENT, FALSE);

return STATUS_MORE_PROCESSING_REQUIRED;
}


//VOID MyDeleteToLinkSet(PFILE_OBJECT fileobject,PKSPIN_LOCK spn)
//{

//}
NTSTATUS SfCleanupClose( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
PAGED_CODE();
ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));

if(DeviceObject==gSFilterControlDeviceObject)     //如果是对控制设置进行清理则直接返回
{
	Irp->IoStatus.Information=0;
	Irp->IoStatus.Status=STATUS_SUCCESS;
	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
IoSkipCurrentIrpStackLocation( Irp );
return IoCallDriver( ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}



NTSTATUS SfFsControl( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
/*
参数说明:

DeviceObject: 我们创建的设备对象。它是被绑定到文件系统控制设备对象栈上。

*/

PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
PAGED_CODE();
ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
switch (irpSp->MinorFunction) {

case IRP_MN_MOUNT_VOLUME:         //物理设备的挂载触发的

return SfFsControlMountVolume( DeviceObject, Irp );

case IRP_MN_LOAD_FILE_SYSTEM:     //文件系统的首次激活时

return SfFsControlLoadFileSystem( DeviceObject, Irp );

case IRP_MN_USER_FS_REQUEST:
{
switch (irpSp->Parameters.FileSystemControl.FsControlCode) {

case FSCTL_DISMOUNT_VOLUME://物理设备的移除，但是很难捕捉到   
{
PSFILTER_DEVICE_EXTENSION devExt =(PSFILTER_DEVICE_EXTENSION)(DeviceObject->DeviceExtension);

SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfFsControl: Dismounting volume %p \"%wZ\"\n",
devExt->AttachedToDeviceObject,
&devExt->DeviceName) );
break;
}
}
break;
}
}

IoSkipCurrentIrpStackLocation( Irp );
return IoCallDriver( ((PSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}



NTSTATUS SfFsControlCompletion( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context )
{
UNREFERENCED_PARAMETER( DeviceObject );
UNREFERENCED_PARAMETER( Irp );

ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
ASSERT(Context != NULL);

#if WINVER >= 0x0501
if (IS_WINDOWSXP_OR_LATER())
{
KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);
}
else
{
#endif
if (KeGetCurrentIrql() > PASSIVE_LEVEL)
{
ExQueueWorkItem( (PWORK_QUEUE_ITEM) Context, DelayedWorkQueue );
}
else
{
PWORK_QUEUE_ITEM workItem = (PWORK_QUEUE_ITEM)Context;
(workItem->WorkerRoutine)(workItem->Parameter);
}

#if WINVER >= 0x0501
}
#endif

return STATUS_MORE_PROCESSING_REQUIRED;
}




NTSTATUS SfFsControlMountVolume( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
/*
参数说明:

DeviceObject: 它是我们创建的设备对象。它被绑定到文件系统CDO的设备栈上。
Irp: 它是发送给文件系统CDO的挂载请求。它是一个新卷的挂载请求。

*/

PSFILTER_DEVICE_EXTENSION devExt = (PSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

PDEVICE_OBJECT storageStackDeviceObject;


// newDeviceObject是将要绑定到文件系统的卷设备对象上。或者说，这个newDeviceObject要被绑定到新挂载卷的设备卷上。
PDEVICE_OBJECT newDeviceObject;

PSFILTER_DEVICE_EXTENSION newDevExt;
NTSTATUS status;
BOOLEAN isShadowCopyVolume;
PFSCTRL_COMPLETION_CONTEXT completionContext;


PAGED_CODE();

ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
ASSERT(IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType));


/*
在把IRP发送到文件系统之前，当挂载请求到来时，Vpb->RealDevice保存的是：将要被挂载的磁盘设备对象。
storageStackDeviceObject事先保存了VPB的值，这是因为：当IRP下发给底层驱动后，可能会改变。
*/
storageStackDeviceObject = irpSp->Parameters.MountVolume.Vpb->RealDevice;


status = SfIsShadowCopyVolume ( storageStackDeviceObject, &isShadowCopyVolume );

if (NT_SUCCESS(status) &&
isShadowCopyVolume &&
!FlagOn(SfDebug,SFDEBUG_ATTACH_TO_SHADOW_COPIES))
{

UNICODE_STRING shadowDeviceName;
WCHAR shadowNameBuffer[MAX_DEVNAME_LENGTH];

RtlInitEmptyUnicodeString( &shadowDeviceName, shadowNameBuffer, sizeof(shadowNameBuffer) );
SfGetObjectName( storageStackDeviceObject, &shadowDeviceName );
SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfFsControlMountVolume Not attaching to Volume %p \"%wZ\", shadow copy volume\n",
storageStackDeviceObject,
&shadowDeviceName) );

//如果不打算绑定卷影则跳到下一层驱动
IoSkipCurrentIrpStackLocation( Irp );
return IoCallDriver( devExt->AttachedToDeviceObject, Irp );
}

status = IoCreateDevice( gSFilterDriverObject,
sizeof( SFILTER_DEVICE_EXTENSION ),
NULL,
DeviceObject->DeviceType,
0,
FALSE,
&newDeviceObject );



/*如果不把IRP发送到文件系统中，那么文件系统就不会收到这个卷的挂载请求。*/
if (!NT_SUCCESS( status ))
{
KdPrint(( "SFilter!SfFsControlMountVolume: Error creating volume device object, status=%08x\n", status ));

Irp->IoStatus.Information = 0;
Irp->IoStatus.Status = status;
IoCompleteRequest( Irp, IO_NO_INCREMENT );

return status;
}


//填写设备扩展，这样目的是：可以让完成函数更容易到storageStackDeviceObject
newDevExt =(PSFILTER_DEVICE_EXTENSION) newDeviceObject->DeviceExtension;
newDevExt->StorageStackDeviceObject = storageStackDeviceObject;
RtlInitEmptyUnicodeString( &newDevExt->DeviceName, newDevExt->DeviceNameBuffer, sizeof(newDevExt->DeviceNameBuffer) );
SfGetObjectName( storageStackDeviceObject, &newDevExt->DeviceName );

#if WINVER >= 0x0501

if (IS_WINDOWSXP_OR_LATER())
{

//在这里设置了事件对象，把它用在完成例程中。这样做的目的是：通知当前例程，文件系统已经完成了当前卷的挂载。
KEVENT waitEvent;
KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
IoCopyCurrentIrpStackLocationToNext ( Irp );
IoSetCompletionRoutine( Irp,
SfFsControlCompletion,
&waitEvent, //context parameter
TRUE,
TRUE,
TRUE );
status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );
if (STATUS_PENDING == status)
{

//等待更底层的驱动完成，然后就会调用完成例程。
status = KeWaitForSingleObject( &waitEvent,
Executive,
KernelMode,
FALSE,
NULL );
ASSERT( STATUS_SUCCESS == status );
}
ASSERT(KeReadStateEvent(&waitEvent) ||
!NT_SUCCESS(Irp->IoStatus.Status));


//执行到了这里，说明卷的挂载已经完成，要开始绑定卷了。等到完成函数设置了事件之后，再来绑定卷。
status = SfFsControlMountVolumeComplete( DeviceObject, Irp, newDeviceObject );

}
else
{
#endif
completionContext = (PFSCTRL_COMPLETION_CONTEXT)ExAllocatePoolWithTag( NonPagedPool, sizeof( FSCTRL_COMPLETION_CONTEXT ), SFLT_POOL_TAG );
if (completionContext == NULL)
{
IoSkipCurrentIrpStackLocation( Irp );
status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );
}
else
{
//初始化一个工作任务。指定一个有待执行的函数SfFsControlMountVolumeCompleteWorker，把这个函数插入到某个线程中去执行。
ExInitializeWorkItem( &completionContext->WorkItem, (PWORKER_THREAD_ROUTINE)SfFsControlMountVolumeCompleteWorker, completionContext );
completionContext->DeviceObject = DeviceObject;
completionContext->Irp = Irp;
completionContext->NewDeviceObject = newDeviceObject;
IoCopyCurrentIrpStackLocationToNext( Irp );
IoSetCompletionRoutine( Irp,
SfFsControlCompletion,
&completionContext->WorkItem, //context parameter
TRUE,
TRUE,
TRUE );
status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );
}
#if WINVER >= 0x0501
}
#endif

return status;
}


VOID SfFsControlMountVolumeCompleteWorker( IN PFSCTRL_COMPLETION_CONTEXT Context )
{
ASSERT( Context != NULL );

SfFsControlMountVolumeComplete( Context->DeviceObject,
Context->Irp,
Context->NewDeviceObject );

ExFreePoolWithTag( Context, SFLT_POOL_TAG );
}



NTSTATUS SfFsControlMountVolumeComplete( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PDEVICE_OBJECT NewDeviceObject )
{
/*
参数说明:

DeviceObject: 它是绑定到文件系统控制设备对象的设备栈上，它是一个过滤设备对象。
Irp: 它是发送给文件系统CDO的挂载请求。它是一个新卷的挂载请求。
NewDeviceObject: 它是新创建的过滤设备对象，用于绑定到文件系统的卷设备对象的设备栈上。

*/

PVPB vpb;
PSFILTER_DEVICE_EXTENSION newDevExt;
PIO_STACK_LOCATION irpSp;
PDEVICE_OBJECT attachedDeviceObject;
NTSTATUS status;

PAGED_CODE();

newDevExt =(PSFILTER_DEVICE_EXTENSION)NewDeviceObject->DeviceExtension;
irpSp = IoGetCurrentIrpStackLocation( Irp );

/*
* 获取我们保存的VPB，这个时候就可以通过该设备对象得到VPB
* VPB->DeviceObject是 文件系统创建的卷设备对象
* VPB->RealDevice是 磁盘驱动创建的物理设备对象
*/
vpb = newDevExt->StorageStackDeviceObject->Vpb;
if (vpb != irpSp->Parameters.MountVolume.Vpb)
{
SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfFsControlMountVolume: VPB in IRP stack changed %p IRPVPB=%p VPB=%p\n",
vpb->DeviceObject,
irpSp->Parameters.MountVolume.Vpb,
vpb) );
}

if (NT_SUCCESS( Irp->IoStatus.Status ))
{
ExAcquireFastMutex( &gSfilterAttachLock );
if (!SfIsAttachedToDevice( vpb->DeviceObject, &attachedDeviceObject ))
{

/*
* SfAttachToMountedDevice的意义：将我们创建的过滤设备对象NewDeviceObject绑定到文件系统创建的VPB->DeviceObject的设备对象栈上。
*/
status = SfAttachToMountedDevice( vpb->DeviceObject, NewDeviceObject );

if (!NT_SUCCESS( status ))
{
SfCleanupMountedDevice( NewDeviceObject );
IoDeleteDevice( NewDeviceObject );
}

ASSERT( NULL == attachedDeviceObject );
}
else
{
SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfFsControlMountVolume Mount volume failure for %p \"%wZ\", already attached\n",
((PSFILTER_DEVICE_EXTENSION)attachedDeviceObject->DeviceExtension)->AttachedToDeviceObject,
&newDevExt->DeviceName) );

SfCleanupMountedDevice( NewDeviceObject );
IoDeleteDevice( NewDeviceObject );
ObDereferenceObject( attachedDeviceObject );
}

ExReleaseFastMutex( &gSfilterAttachLock );
}
else
{
SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfFsControlMountVolume: Mount volume failure for %p \"%wZ\", status=%08x\n",
DeviceObject,
&newDevExt->DeviceName,
Irp->IoStatus.Status) );
SfCleanupMountedDevice( NewDeviceObject );
IoDeleteDevice( NewDeviceObject );
}

status = Irp->IoStatus.Status;
IoCompleteRequest( Irp, IO_NO_INCREMENT );
return status;
}



NTSTATUS SfFsControlLoadFileSystem( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
PSFILTER_DEVICE_EXTENSION devExt =(PSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
NTSTATUS status;
PFSCTRL_COMPLETION_CONTEXT completionContext;

PAGED_CODE();

SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfFscontrolLoadFileSystem: Loading File System, Detaching from \"%wZ\"\n",
&devExt->DeviceName) );

#if WINVER >= 0x0501
if (IS_WINDOWSXP_OR_LATER())
{

KEVENT waitEvent;

KeInitializeEvent( &waitEvent,
NotificationEvent,
FALSE );

IoCopyCurrentIrpStackLocationToNext( Irp );

IoSetCompletionRoutine( Irp,
SfFsControlCompletion,
&waitEvent, //context parameter
TRUE,
TRUE,
TRUE );

status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

if (STATUS_PENDING == status)
{

status = KeWaitForSingleObject( &waitEvent,
Executive,
KernelMode,
FALSE,
NULL );
ASSERT( STATUS_SUCCESS == status );
}

ASSERT(KeReadStateEvent(&waitEvent) ||
!NT_SUCCESS(Irp->IoStatus.Status));

status = SfFsControlLoadFileSystemComplete( DeviceObject, Irp );

} else {
#endif

completionContext =(PFSCTRL_COMPLETION_CONTEXT) ExAllocatePoolWithTag( NonPagedPool,
sizeof( FSCTRL_COMPLETION_CONTEXT ),
SFLT_POOL_TAG );

if (completionContext == NULL) {

IoSkipCurrentIrpStackLocation( Irp );
status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

} else {

ExInitializeWorkItem( &completionContext->WorkItem, (PWORKER_THREAD_ROUTINE)SfFsControlMountVolumeCompleteWorker, completionContext );

completionContext->DeviceObject = DeviceObject;
completionContext->Irp = Irp;
completionContext->NewDeviceObject = NULL;

IoCopyCurrentIrpStackLocationToNext( Irp );

IoSetCompletionRoutine(
Irp,
SfFsControlCompletion,
completionContext,
TRUE,
TRUE,
TRUE );

IoDetachDevice( devExt->AttachedToDeviceObject );
status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );
}
#if WINVER >= 0x0501
}
#endif

return status;
}



VOID SfFsControlLoadFileSystemCompleteWorker( IN PFSCTRL_COMPLETION_CONTEXT Context )
{
ASSERT( NULL != Context );
SfFsControlLoadFileSystemComplete( Context->DeviceObject, Context->Irp );
ExFreePoolWithTag( Context, SFLT_POOL_TAG );
}


NTSTATUS SfFsControlLoadFileSystemComplete ( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
PSFILTER_DEVICE_EXTENSION devExt;
NTSTATUS status;

PAGED_CODE();

devExt = (PSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfFsControlLoadFileSystem: Detaching from recognizer %p \"%wZ\", status=%08x\n",
DeviceObject,
&devExt->DeviceName,
Irp->IoStatus.Status) );
if (!NT_SUCCESS( Irp->IoStatus.Status ) && (Irp->IoStatus.Status != STATUS_IMAGE_ALREADY_LOADED))
{
SfAttachDeviceToDeviceStack( DeviceObject, devExt->AttachedToDeviceObject, &devExt->AttachedToDeviceObject );
ASSERT(devExt->AttachedToDeviceObject != NULL);
}
else
{
SfCleanupMountedDevice( DeviceObject );
IoDeleteDevice( DeviceObject );
}

status = Irp->IoStatus.Status;
IoCompleteRequest( Irp, IO_NO_INCREMENT );
return status;
}



/*****************************************************************************************************************************************************
以下的代码是FastIO 的处理函数:

由于我们的驱动将要绑定到文件系统驱动的上边，文件系统除了处理正常的IRP之外，还要处理所谓的FastIo
FastIo是Cache Manager调用所引发的一种没有irp的请求。除了正常的Dispatch Functions之外，你还得为DriverObject 编写另一组Fast Io Functions。
文件系统的普通分发例程和fastio例程都随时有可能被调用。做好的过滤驱动显然应该同时过滤这两套接口。

在进行基于IRP为基础的接口调用前, IO管理器会尝试使用FAST IO 接口来加速各种IO操作。


FastIO 的设计是用来处理快速的、同步的、并且“on cached files”的IO操作。当进行FastIO操作时，所需处理的数据是直接在用户buffer和系统缓存中进行传输的，
而不是通过文件系统和存储器驱动栈(storage driver stack)。
事实上存储器驱动并不使用FastIO机制。当需要处理的数据已经存在于系统缓存，则采用FastIO机制的读写操作立刻就可以完成。否则，系统会产生一个缺页中断，
这会导致系统发送相应的IRP包来完成用户所需的读写操作。通常发生这种情况（指：所需的数据不在系统缓存的情况的时候，FastIO函数会返回FALSE，
或者一直等到缺页中端响应函数把所需的数据都加载到系统缓存中。(如果FastIO处理函数返回FALSE，那么调用者必须自己创建相应Irp来完成所需的处理。)


文件系统过滤驱动程序的控制设备对象（CDO）并不一定需要处理IO操作。过滤器设备对象(FiDO)则需要将所有不能识别的Irp包都传递到自己下层的驱动程序。
另外有一点需要注意，挂在到卷上的过滤器设备对象必须提供 FastIoDetachDevice 函数。

******************************************************************************************************************************************************/
BOOLEAN SfFastIoCheckIfPossible( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, IN BOOLEAN CheckForReadOperation, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoCheckIfPossible ))
{
return (fastIoDispatch->FastIoCheckIfPossible)(
FileObject,
FileOffset,
Length,
Wait,
LockKey,
CheckForReadOperation,
IoStatus,
nextDeviceObject );
}
}
return FALSE;
}
BOOLEAN SfFastIoRead( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, OUT PVOID Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension) {

ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoRead ))
{
return (fastIoDispatch->FastIoRead)(
FileObject,
FileOffset,
Length,
Wait,
LockKey,
Buffer,
IoStatus,
nextDeviceObject );
}
}
return FALSE;
}
BOOLEAN SfFastIoWrite( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, IN PVOID Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWrite ))
{
return (fastIoDispatch->FastIoWrite)(
FileObject,
FileOffset,
Length,
Wait,
LockKey,
Buffer,
IoStatus,
nextDeviceObject );
}
}
return FALSE;
}
BOOLEAN SfFastIoQueryBasicInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, OUT PFILE_BASIC_INFORMATION Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryBasicInfo ))
{
return (fastIoDispatch->FastIoQueryBasicInfo)(
FileObject,
Wait,
Buffer,
IoStatus,
nextDeviceObject );
}
}
return FALSE;
}
BOOLEAN SfFastIoQueryStandardInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, OUT PFILE_STANDARD_INFORMATION Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryStandardInfo ))
{
return (fastIoDispatch->FastIoQueryStandardInfo)(
FileObject,
Wait,
Buffer,
IoStatus,
nextDeviceObject );
}
}
return FALSE;
}
BOOLEAN SfFastIoLock( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PLARGE_INTEGER Length, PEPROCESS ProcessId, ULONG Key, BOOLEAN FailImmediately, BOOLEAN ExclusiveLock, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoLock ))
{
return (fastIoDispatch->FastIoLock)(
FileObject,
FileOffset,
Length,
ProcessId,
Key,
FailImmediately,
ExclusiveLock,
IoStatus,
nextDeviceObject );
}
}
return FALSE;
}
BOOLEAN SfFastIoUnlockSingle( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PLARGE_INTEGER Length, PEPROCESS ProcessId, ULONG Key, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockSingle ))
{
return (fastIoDispatch->FastIoUnlockSingle)(
FileObject,
FileOffset,
Length,
ProcessId,
Key,
IoStatus,
nextDeviceObject );
}
}
return FALSE;
}
BOOLEAN SfFastIoUnlockAll( IN PFILE_OBJECT FileObject, PEPROCESS ProcessId, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
if (nextDeviceObject)
{
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAll ))
{
return (fastIoDispatch->FastIoUnlockAll)(
FileObject,
ProcessId,
IoStatus,
nextDeviceObject );
}
}
}
return FALSE;
}
BOOLEAN SfFastIoUnlockAllByKey( IN PFILE_OBJECT FileObject, PVOID ProcessId, ULONG Key, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAllByKey ))
{
return (fastIoDispatch->FastIoUnlockAllByKey)(
FileObject,
ProcessId,
Key,
IoStatus,
nextDeviceObject );
}
}
return FALSE;
}


BOOLEAN SfFastIoDeviceControl( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, IN PVOID InputBuffer OPTIONAL, IN ULONG InputBufferLength, OUT PVOID OutputBuffer OPTIONAL, IN ULONG OutputBufferLength, IN ULONG IoControlCode, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;
PAGED_CODE();

if (DeviceObject!=gSFilterControlDeviceObject)//如果不是控制设备那么就简单的返回false，系统会重新分配irp
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryNetworkOpenInfo ))
{
	return (fastIoDispatch->FastIoDeviceControl)(
FileObject,
Wait,
InputBuffer,
InputBufferLength,
OutputBuffer,
OutputBufferLength,
IoControlCode,
IoStatus,
DeviceObject);
}
return FALSE;
}
KdPrint(("fastio:device io control!\n"));
return FALSE;

//PAGED_CODE();
UNREFERENCED_PARAMETER(DeviceObject);
UNREFERENCED_PARAMETER(Wait);
UNREFERENCED_PARAMETER(FileObject);
NTSTATUS status=STATUS_SUCCESS;


switch(IoControlCode)
{
  	case(IOCTL_DEVICE_READ):
		break;
	case(IOCTL_DEVICE_WRITE):

		break;
	case(IOCTL_VERIFY_OPERATION_INFORMATION)://
	    KdPrint(("收到了来自应用的取操作信息通知！\n"));
		ASSERT(InputBufferLength==sizeof(POperationRecord));
        ASSERT(OutputBufferLength==sizeof(POperationRecord));
		status=FastIO_GetOperationInformation(IOCTL_VERIFY_OPERATION_INFORMATION,InputBuffer,OutputBuffer,InputBufferLength,OutputBufferLength);
		break;
	case(IOCTL_SEND_VERIFY_RESULT):
		KdPrint(("收到了来自应用程序的操作验证结果通知!\n"));		
		status=FastIO_SetOperationResult(IOCTL_SEND_VERIFY_RESULT,InputBuffer,OutputBuffer,InputBufferLength,OutputBufferLength);
		break;
	default:
		KdPrint(("UnKown Device Control Code:%d!",IoControlCode));
		break;
}
/*if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoDeviceControl ))
{
return (fastIoDispatch->FastIoDeviceControl)(
FileObject,
Wait,
InputBuffer,
InputBufferLength,
OutputBuffer,
OutputBufferLength,
IoControlCode,
IoStatus,
nextDeviceObject );
}
}*/
IoStatus->Status=status;
IoStatus->Information=0;

return TRUE;
}


VOID SfFastIoDetachDevice( IN PDEVICE_OBJECT SourceDevice, IN PDEVICE_OBJECT TargetDevice )
{
PSFILTER_DEVICE_EXTENSION devExt;

PAGED_CODE();

ASSERT(IS_MY_DEVICE_OBJECT( SourceDevice ));
devExt =(PSFILTER_DEVICE_EXTENSION)SourceDevice->DeviceExtension;
SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfFastIoDetachDevice: Detaching from volume %p \"%wZ\"\n",
TargetDevice, &devExt->DeviceName) );
SfCleanupMountedDevice( SourceDevice );
IoDetachDevice( TargetDevice );
IoDeleteDevice( SourceDevice );
}


BOOLEAN SfFastIoQueryNetworkOpenInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, OUT PFILE_NETWORK_OPEN_INFORMATION Buffer, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryNetworkOpenInfo ))
{
return (fastIoDispatch->FastIoQueryNetworkOpenInfo)(
FileObject,
Wait,
Buffer,
IoStatus,
nextDeviceObject );
}
}
return FALSE;
}


BOOLEAN SfFastIoMdlRead( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlRead ))
{
return (fastIoDispatch->MdlRead)(
FileObject,
FileOffset,
Length,
LockKey,
MdlChain,
IoStatus,
nextDeviceObject );
}
}
return FALSE;
}


BOOLEAN SfFastIoMdlReadComplete( IN PFILE_OBJECT FileObject, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadComplete ))
{
return (fastIoDispatch->MdlReadComplete)( FileObject, MdlChain, nextDeviceObject );
}
}
return FALSE;
}


BOOLEAN SfFastIoPrepareMdlWrite( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, PrepareMdlWrite ))
{
return (fastIoDispatch->PrepareMdlWrite)(
FileObject,
FileOffset,
Length,
LockKey,
MdlChain,
IoStatus,
nextDeviceObject );
}
}
return FALSE;
}


BOOLEAN SfFastIoMdlWriteComplete( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteComplete ))
{
return (fastIoDispatch->MdlWriteComplete)(
FileObject,
FileOffset,
MdlChain,
nextDeviceObject );
}
}
return FALSE;
}


BOOLEAN SfFastIoReadCompressed( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, OUT PVOID Buffer, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo, IN ULONG CompressedDataInfoLength, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoReadCompressed ))
{
return (fastIoDispatch->FastIoReadCompressed)(
FileObject,
FileOffset,
Length,
LockKey,
Buffer,
MdlChain,
IoStatus,
CompressedDataInfo,
CompressedDataInfoLength,
nextDeviceObject );
}
}
return FALSE;
}



BOOLEAN SfFastIoWriteCompressed( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey, IN PVOID Buffer, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus, IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo, IN ULONG CompressedDataInfoLength, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWriteCompressed ))
{
return (fastIoDispatch->FastIoWriteCompressed)(
FileObject,
FileOffset,
Length,
LockKey,
Buffer,
MdlChain,
IoStatus,
CompressedDataInfo,
CompressedDataInfoLength,
nextDeviceObject );
}
}
return FALSE;
}



BOOLEAN SfFastIoMdlReadCompleteCompressed( IN PFILE_OBJECT FileObject, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadCompleteCompressed ))
{
return (fastIoDispatch->MdlReadCompleteCompressed)(
FileObject,
MdlChain,
nextDeviceObject );
}
}
return FALSE;
}



BOOLEAN SfFastIoMdlWriteCompleteCompressed( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;

if (DeviceObject->DeviceExtension) {

ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);

fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteCompleteCompressed )) {

return (fastIoDispatch->MdlWriteCompleteCompressed)(
FileObject,
FileOffset,
MdlChain,
nextDeviceObject );
}
}
return FALSE;
}



BOOLEAN SfFastIoQueryOpen( IN PIRP Irp, OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation, IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT nextDeviceObject;
PFAST_IO_DISPATCH fastIoDispatch;
BOOLEAN result;

PAGED_CODE();

if (DeviceObject->DeviceExtension)
{
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
ASSERT(nextDeviceObject);
fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;
if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryOpen ))
{
PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
irpSp->DeviceObject = nextDeviceObject;
result = (fastIoDispatch->FastIoQueryOpen)(
Irp,
NetworkInformation,
nextDeviceObject );

irpSp->DeviceObject = DeviceObject;
return result;
}
}
return FALSE;
}



#if WINVER >= 0x0501
//========================== FSFilter 回调函数 ===========================
NTSTATUS SfPreFsFilterPassThrough( IN PFS_FILTER_CALLBACK_DATA Data, OUT PVOID *CompletionContext )
{
UNREFERENCED_PARAMETER( Data );
UNREFERENCED_PARAMETER( CompletionContext );

ASSERT( IS_MY_DEVICE_OBJECT( Data->DeviceObject ) );

return STATUS_SUCCESS;
}


VOID SfPostFsFilterPassThrough ( IN PFS_FILTER_CALLBACK_DATA Data, IN NTSTATUS OperationStatus, IN PVOID CompletionContext )
{
UNREFERENCED_PARAMETER( Data );
UNREFERENCED_PARAMETER( OperationStatus );
UNREFERENCED_PARAMETER( CompletionContext );

ASSERT( IS_MY_DEVICE_OBJECT( Data->DeviceObject ) );
}
#endif



NTSTATUS SfAttachDeviceToDeviceStack( IN PDEVICE_OBJECT SourceDevice, IN PDEVICE_OBJECT TargetDevice, IN OUT PDEVICE_OBJECT *AttachedToDeviceObject )
{
/*
参数说明:

SourceDevice: 它是调用者创建的设备对象。比如调用者使用IoCreateDevice函数来创建设备对象。
TargetDevice: 它指向其它驱动程序的设备对象。
AttachedToDeviceObject: 它用来存储IoAttachDeviceToDeviceStack函数的返回值。

*/

PAGED_CODE();

#if WINVER >= 0x0501

if (IS_WINDOWSXP_OR_LATER())
{
ASSERT( NULL != gSfDynamicFunctions.AttachDeviceToDeviceStackSafe );
return (gSfDynamicFunctions.AttachDeviceToDeviceStackSafe)( SourceDevice, TargetDevice, AttachedToDeviceObject );
}
else
{
ASSERT( NULL == gSfDynamicFunctions.AttachDeviceToDeviceStackSafe );
#endif

*AttachedToDeviceObject = TargetDevice;
*AttachedToDeviceObject = IoAttachDeviceToDeviceStack( SourceDevice, TargetDevice );

if (*AttachedToDeviceObject == NULL)
{
return STATUS_NO_SUCH_DEVICE;
}

return STATUS_SUCCESS;

#if WINVER >= 0x0501
}
#endif
}



NTSTATUS SfAttachToFileSystemDevice( IN PDEVICE_OBJECT DeviceObject, IN PUNICODE_STRING DeviceName )
{
/*
SfAttachToFileSystemDevice函数用来完成对文件系统控制设备的绑定。

参数说明:
DeviceObject: 它指向文件系统的控制设备对象(CDO)。即 被激活或则撤消的File System CDO

*/

PDEVICE_OBJECT newDeviceObject; //新设备对象
PSFILTER_DEVICE_EXTENSION devExt; //文件系统过滤驱动定义的设备扩展
NTSTATUS status; //状态码
UNICODE_STRING fsrecName;
UNICODE_STRING fsName; //文件系统名
WCHAR tempNameBuffer[MAX_DEVNAME_LENGTH];//临时缓冲区(存放名字串)

PAGED_CODE();

if (!IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType)) //测试给定设备是不是所需要关心的设备
{
return STATUS_SUCCESS;
}


/*
* Windows的标准文件系统识别器基本上都是由驱动 \FileSystem\Fs_Rec 生成的。所以直接判断驱动的名字可以解决一部分问题。
* 也不一定非得要把文件系统识别器生成在驱动\FileSystem\Fs_Rec下面。只是说，一般情况下是在\FileSystem\Fs_Rec下面。
*/
RtlInitEmptyUnicodeString( &fsName, tempNameBuffer, sizeof(tempNameBuffer) );

if (!FlagOn(SfDebug,SFDEBUG_ATTACH_TO_FSRECOGNIZER))
{
RtlInitUnicodeString( &fsrecName, L"\\FileSystem\\Fs_Rec" );
SfGetObjectName( DeviceObject->DriverObject, &fsName );

if (RtlCompareUnicodeString( &fsName, &fsrecName, TRUE ) == 0)
{
//通过驱动的名字来分辨出部分Windows的标准文件系统识别器。如果是，那么返回成功，也就是放弃绑定了。
//如果，有错过的文件系统识别器没有被判断到，文件系统控制请求的过滤驱动中有对应的处理。
return STATUS_SUCCESS;
}
}



//是我们关心的文件系统，且不是微软的文件系统识别器的设备，创建一个设备绑定这个设备对象。
status = IoCreateDevice( gSFilterDriverObject,
sizeof( SFILTER_DEVICE_EXTENSION ),


NULL,
DeviceObject->DeviceType,
0,
FALSE,
&newDeviceObject );

if (!NT_SUCCESS( status ))
{
return status;
}

if ( FlagOn( DeviceObject->Flags, DO_BUFFERED_IO ))
{
SetFlag( newDeviceObject->Flags, DO_BUFFERED_IO );
}

if ( FlagOn( DeviceObject->Flags, DO_DIRECT_IO ))
{
SetFlag( newDeviceObject->Flags, DO_DIRECT_IO );
}

if ( FlagOn( DeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN ) )
{
SetFlag( newDeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN );
}

devExt =(PSFILTER_DEVICE_EXTENSION) newDeviceObject->DeviceExtension;


/*
调用SfAttachDeviceToDeviceStack函数将过滤设备对象绑定到File System CDO的设备栈上面。这样，我们的newDeviceObject就可以接收到发送到
File System CDO的IRP_MJ_FILE_SYSTEM_CONTROL请求了。 以后，程序就可以去绑定卷了。
使用SfAttachDeviceToDeviceStack函数来进行绑定。参数1绑定到参数2，绑定函数返回的设备存储在参数3中。
*/
status = SfAttachDeviceToDeviceStack( newDeviceObject, DeviceObject, &devExt->AttachedToDeviceObject );
if (!NT_SUCCESS( status ))
{
goto ErrorCleanupDevice;
}

RtlInitEmptyUnicodeString( &devExt->DeviceName, devExt->DeviceNameBuffer, sizeof(devExt->DeviceNameBuffer) );
RtlCopyUnicodeString( &devExt->DeviceName, DeviceName ); //Save Name
ClearFlag( newDeviceObject->Flags, DO_DEVICE_INITIALIZING );
SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfAttachToFileSystemDevice: Attaching to file system %p \"%wZ\" (%s)\n",
DeviceObject,
&devExt->DeviceName,
GET_DEVICE_TYPE_NAME(newDeviceObject->DeviceType)) );

#if WINVER >= 0x0501

if (IS_WINDOWSXP_OR_LATER())
{
ASSERT( NULL != gSfDynamicFunctions.EnumerateDeviceObjectList &&
NULL != gSfDynamicFunctions.GetDiskDeviceObject &&
NULL != gSfDynamicFunctions.GetDeviceAttachmentBaseRef &&
NULL != gSfDynamicFunctions.GetLowerDeviceObject );


/*
函数SpyEnumerateFileSystemVolumes枚举给定的文件系统下的当前存在的所有挂载了的设备，并且绑定他们。
这样做的目的，是因为过滤驱动可能随时被加载，但是加载过滤驱动的时候，文件系统已经挂载了卷设备。
既是：让过滤驱动加后载，随时都能绑定已经存在或刚刚挂载上来的文件系统卷设备。
*/
status = SfEnumerateFileSystemVolumes( DeviceObject, &fsName );
if (!NT_SUCCESS( status ))
{
IoDetachDevice( devExt->AttachedToDeviceObject );
goto ErrorCleanupDevice;
}
}

#endif

return STATUS_SUCCESS;

ErrorCleanupDevice:
SfCleanupMountedDevice( newDeviceObject );
IoDeleteDevice( newDeviceObject );

return status;
}



VOID SfDetachFromFileSystemDevice( IN PDEVICE_OBJECT DeviceObject )
{
PDEVICE_OBJECT ourAttachedDevice;
PSFILTER_DEVICE_EXTENSION devExt;

PAGED_CODE();

ourAttachedDevice = DeviceObject->AttachedDevice;
while (NULL != ourAttachedDevice)
{
if (IS_MY_DEVICE_OBJECT( ourAttachedDevice ))
{
devExt =(PSFILTER_DEVICE_EXTENSION)ourAttachedDevice->DeviceExtension;
SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfDetachFromFileSystemDevice: Detaching from file system %p \"%wZ\" (%s)\n",
devExt->AttachedToDeviceObject,
&devExt->DeviceName,
GET_DEVICE_TYPE_NAME(ourAttachedDevice->DeviceType)) );

SfCleanupMountedDevice( ourAttachedDevice );
IoDetachDevice( DeviceObject );
IoDeleteDevice( ourAttachedDevice );

return;
}

DeviceObject = ourAttachedDevice;
ourAttachedDevice = ourAttachedDevice->AttachedDevice;
}
}



#if WINVER >= 0x0501
NTSTATUS SfEnumerateFileSystemVolumes( IN PDEVICE_OBJECT FSDeviceObject, IN PUNICODE_STRING Name )
{
/*
参数说明:

FSDeviceObject: 它指向文件系统的控制设备对象(CDO)。即 被激活或则撤消的File System CDO
Name: 它是文件系统的名字

*/

PDEVICE_OBJECT newDeviceObject;
PSFILTER_DEVICE_EXTENSION newDevExt;
PDEVICE_OBJECT *devList;
PDEVICE_OBJECT storageStackDeviceObject;
NTSTATUS status;
ULONG numDevices;
ULONG i;
BOOLEAN isShadowCopyVolume;

PAGED_CODE();

/*
* IoEnumerateDeviceObjectList函数枚举这个驱动下的设备对象列表。这个函数将被调用2次。
* 第1次调用： 获取设备列表中的设备对象的数量。
* 第2次调用: 根据第1次的结果numDevices值来开辟设备对象的存放空间，从而得到设备链devList。
*/
status = (gSfDynamicFunctions.EnumerateDeviceObjectList)(
FSDeviceObject->DriverObject,
NULL,
0,
&numDevices);

if (!NT_SUCCESS( status ))
{
ASSERT(STATUS_BUFFER_TOO_SMALL == status);
numDevices += 8; //为已知的设备开辟内存空间进行存储。额外增加8字节。


devList =(PDEVICE_OBJECT *)ExAllocatePoolWithTag( NonPagedPool, (numDevices * sizeof(PDEVICE_OBJECT)), SFLT_POOL_TAG );
if (NULL == devList)
{
return STATUS_INSUFFICIENT_RESOURCES;
}

ASSERT( NULL != gSfDynamicFunctions.EnumerateDeviceObjectList );
status = (gSfDynamicFunctions.EnumerateDeviceObjectList)(
FSDeviceObject->DriverObject,
devList,
(numDevices * sizeof(PDEVICE_OBJECT)),
&numDevices);

if (!NT_SUCCESS( status ))
{
ExFreePool( devList );
return status;
}

//依次遍历各个设备对象
for (i=0; i < numDevices; i++)
{
storageStackDeviceObject = NULL;
_try {

//如果设备对象是文件系统CDO，或者是不符合类型的，或者是已经绑定的
if ((devList[i] == FSDeviceObject) || (devList[i]->DeviceType != FSDeviceObject->DeviceType) || SfIsAttachedToDevice( devList[i], NULL ))
{
_leave;
}

SfGetBaseDeviceObjectName( devList[i], Name );
if (Name->Length > 0)
{
_leave;
}


/*
调用IoGetDiskDeviceObject函数来获取一个与文件系统设备对象有关的磁盘设备对象。只绑定已经拥有一个磁盘设备对象的文件系统设备对象。
*/
ASSERT( NULL != gSfDynamicFunctions.GetDiskDeviceObject );
status = (gSfDynamicFunctions.GetDiskDeviceObject)( devList[i], &storageStackDeviceObject );

if (!NT_SUCCESS( status ))
{
_leave;
}

status = SfIsShadowCopyVolume ( storageStackDeviceObject, &isShadowCopyVolume );

if (NT_SUCCESS(status) &&
isShadowCopyVolume &&
!FlagOn(SfDebug,SFDEBUG_ATTACH_TO_SHADOW_COPIES))
{

UNICODE_STRING shadowDeviceName;
WCHAR shadowNameBuffer[MAX_DEVNAME_LENGTH];

RtlInitEmptyUnicodeString( &shadowDeviceName, shadowNameBuffer, sizeof(shadowNameBuffer) );
SfGetObjectName( storageStackDeviceObject, &shadowDeviceName );

SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfEnumerateFileSystemVolumes Not attaching to Volume %p \"%wZ\", shadow copy volume\n",
storageStackDeviceObject,
&shadowDeviceName) );

_leave;
}


// 是一个磁盘设备对象，创建新的设备对象，准备绑定。
status = IoCreateDevice( gSFilterDriverObject,
sizeof( SFILTER_DEVICE_EXTENSION ),
NULL,
devList[i]->DeviceType,
0,
FALSE,
&newDeviceObject );
if (!NT_SUCCESS( status ))
{
_leave;
}

newDevExt =(PSFILTER_DEVICE_EXTENSION)newDeviceObject->DeviceExtension;
newDevExt->StorageStackDeviceObject = storageStackDeviceObject;
RtlInitEmptyUnicodeString( &newDevExt->DeviceName, newDevExt->DeviceNameBuffer,sizeof(newDevExt->DeviceNameBuffer) );
SfGetObjectName( storageStackDeviceObject, &newDevExt->DeviceName );


/*
在绑定最后的时候，再测试下，该设备是否被绑定过。这里加了一个锁。如果没被绑定，则执行下面的绑定过程，否则，直接返回。
*/
ExAcquireFastMutex( &gSfilterAttachLock );
if (!SfIsAttachedToDevice( devList[i], NULL ))
{
status = SfAttachToMountedDevice( devList[i], newDeviceObject );
if (!NT_SUCCESS( status ))
{
SfCleanupMountedDevice( newDeviceObject );
IoDeleteDevice( newDeviceObject );
}
}
else
{
SfCleanupMountedDevice( newDeviceObject );
IoDeleteDevice( newDeviceObject );
}

ExReleaseFastMutex( &gSfilterAttachLock );
}

_finally {

/*
减少设备对象的计数，这个计数是由函数IoGetDiskDeviceObject增加的。成功绑定后，就减少该设备对象的计数。
一旦成功绑定到devList[i]，I/O管理器会确定设备栈的下层设备，会一直存在，一直到这个文件系统栈被卸掉。
*/
if (storageStackDeviceObject != NULL)
{
ObDereferenceObject( storageStackDeviceObject );
}


//减少设备对象的计数，这个计数是由函数IoEnumerateDeviceObjectList增加的。
ObDereferenceObject( devList[i] );
}
}

status = STATUS_SUCCESS;
ExFreePool( devList );
}

return status;
}
#endif


#if WINVER >= 0x0501
NTSTATUS SfEnumerateFileSystemVolumes( IN PDEVICE_OBJECT FSDeviceObject, IN PUNICODE_STRING Name )
{
/*
参数说明:

FSDeviceObject: 它指向文件系统的控制设备对象(CDO)。即 被激活或则撤消的File System CDO
Name: 它是文件系统的名字



*/

PDEVICE_OBJECT newDeviceObject;
PSFILTER_DEVICE_EXTENSION newDevExt;
PDEVICE_OBJECT *devList;
PDEVICE_OBJECT storageStackDeviceObject;
NTSTATUS status;
ULONG numDevices;
ULONG i;
BOOLEAN isShadowCopyVolume;

PAGED_CODE();

/*
* IoEnumerateDeviceObjectList函数枚举这个驱动下的设备对象列表。这个函数将被调用2次。
* 第1次调用： 获取设备列表中的设备对象的数量。
* 第2次调用: 根据第1次的结果numDevices值来开辟设备对象的存放空间，从而得到设备链devList。
*/
status = (gSfDynamicFunctions.EnumerateDeviceObjectList)(
FSDeviceObject->DriverObject,
NULL,
0,
&numDevices);

if (!NT_SUCCESS( status ))
{
ASSERT(STATUS_BUFFER_TOO_SMALL == status);
numDevices += 8; //为已知的设备开辟内存空间进行存储。额外增加8字节。


devList =(PDEVICE_OBJECT *)ExAllocatePoolWithTag( NonPagedPool, (numDevices * sizeof(PDEVICE_OBJECT)), SFLT_POOL_TAG );
if (NULL == devList)
{
return STATUS_INSUFFICIENT_RESOURCES;
}

ASSERT( NULL != gSfDynamicFunctions.EnumerateDeviceObjectList );
status = (gSfDynamicFunctions.EnumerateDeviceObjectList)(
FSDeviceObject->DriverObject,
devList,
(numDevices * sizeof(PDEVICE_OBJECT)),
&numDevices);

if (!NT_SUCCESS( status ))
{
ExFreePool( devList );
return status;
}

//依次遍历各个设备对象
for (i=0; i < numDevices; i++)
{
storageStackDeviceObject = NULL;
_try {

//如果设备对象是文件系统CDO，或者是不符合类型的，或者是已经绑定的
if ((devList[i] == FSDeviceObject) || (devList[i]->DeviceType != FSDeviceObject->DeviceType) || SfIsAttachedToDevice( devList[i], NULL ))
{
_leave;
}

SfGetBaseDeviceObjectName( devList[i], Name );
if (Name->Length > 0)
{
_leave;
}


/*
调用IoGetDiskDeviceObject函数来获取一个与文件系统设备对象有关的磁盘设备对象。只绑定已经拥有一个磁盘设备对象的文件系统设备对象。
*/
ASSERT( NULL != gSfDynamicFunctions.GetDiskDeviceObject );
status = (gSfDynamicFunctions.GetDiskDeviceObject)( devList[i], &storageStackDeviceObject );

if (!NT_SUCCESS( status ))
{
_leave;
}

status = SfIsShadowCopyVolume ( storageStackDeviceObject, &isShadowCopyVolume );

if (NT_SUCCESS(status) &&
isShadowCopyVolume &&
!FlagOn(SfDebug,SFDEBUG_ATTACH_TO_SHADOW_COPIES))
{

UNICODE_STRING shadowDeviceName;
WCHAR shadowNameBuffer[MAX_DEVNAME_LENGTH];

RtlInitEmptyUnicodeString( &shadowDeviceName, shadowNameBuffer, sizeof(shadowNameBuffer) );
SfGetObjectName( storageStackDeviceObject, &shadowDeviceName );

SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfEnumerateFileSystemVolumes Not attaching to Volume %p \"%wZ\", shadow copy volume\n",
storageStackDeviceObject,
&shadowDeviceName) );

_leave;
}


// 是一个磁盘设备对象，创建新的设备对象，准备绑定。
status = IoCreateDevice( gSFilterDriverObject,
sizeof( SFILTER_DEVICE_EXTENSION ),
NULL,
devList[i]->DeviceType,
0,
FALSE,
&newDeviceObject );
if (!NT_SUCCESS( status ))
{
_leave;
}

newDevExt =(PSFILTER_DEVICE_EXTENSION)newDeviceObject->DeviceExtension;
newDevExt->StorageStackDeviceObject = storageStackDeviceObject;
RtlInitEmptyUnicodeString( &newDevExt->DeviceName, newDevExt->DeviceNameBuffer,sizeof(newDevExt->DeviceNameBuffer) );
SfGetObjectName( storageStackDeviceObject, &newDevExt->DeviceName );


/*
在绑定最后的时候，再测试下，该设备是否被绑定过。这里加了一个锁。如果没被绑定，则执行下面的绑定过程，否则，直接返回。
*/
ExAcquireFastMutex( &gSfilterAttachLock );
if (!SfIsAttachedToDevice( devList[i], NULL ))
{
status = SfAttachToMountedDevice( devList[i], newDeviceObject );
if (!NT_SUCCESS( status ))
{
SfCleanupMountedDevice( newDeviceObject );
IoDeleteDevice( newDeviceObject );
}
}
else
{
SfCleanupMountedDevice( newDeviceObject );
IoDeleteDevice( newDeviceObject );
}

ExReleaseFastMutex( &gSfilterAttachLock );
}

_finally {

/*
减少设备对象的计数，这个计数是由函数IoGetDiskDeviceObject增加的。成功绑定后，就减少该设备对象的计数。
一旦成功绑定到devList[i]，I/O管理器会确定设备栈的下层设备，会一直存在，一直到这个文件系统栈被卸掉。
*/
if (storageStackDeviceObject != NULL)
{
ObDereferenceObject( storageStackDeviceObject );
}


//减少设备对象的计数，这个计数是由函数IoEnumerateDeviceObjectList增加的。
ObDereferenceObject( devList[i] );
}
}

status = STATUS_SUCCESS;
ExFreePool( devList );
}

return status;
}
#endif


NTSTATUS SfAttachToMountedDevice( IN PDEVICE_OBJECT DeviceObject, IN PDEVICE_OBJECT SFilterDeviceObject )
{
/*
SfAttachToMountedDevice函数的功能: 完成绑定一个文件系统卷设备的操作。

参数说明:
SFilterDeviceObject: 它是我们使用IoCreateDevice函数来创建的设备对象。

*/

PSFILTER_DEVICE_EXTENSION newDevExt =(PSFILTER_DEVICE_EXTENSION) SFilterDeviceObject->DeviceExtension;
NTSTATUS status;
ULONG i;

PAGED_CODE();
ASSERT(IS_MY_DEVICE_OBJECT( SFilterDeviceObject ));
#if WINVER >= 0x0501
ASSERT(!SfIsAttachedToDevice ( DeviceObject, NULL ));
#endif

if (FlagOn( DeviceObject->Flags, DO_BUFFERED_IO ))
{
SetFlag( SFilterDeviceObject->Flags, DO_BUFFERED_IO );
}

if (FlagOn( DeviceObject->Flags, DO_DIRECT_IO ))
{
SetFlag( SFilterDeviceObject->Flags, DO_DIRECT_IO );
}

for (i=0; i < 8; i++)
{
LARGE_INTEGER interval;

//调用SfAttachDeviceToDeviceStack函数进行 卷的绑定
status = SfAttachDeviceToDeviceStack( SFilterDeviceObject, DeviceObject, &newDevExt->AttachedToDeviceObject );
if (NT_SUCCESS(status))
{
ClearFlag( SFilterDeviceObject->Flags, DO_DEVICE_INITIALIZING );
SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
("SFilter!SfAttachToMountedDevice: Attaching to volume %p \"%wZ\"\n",
newDevExt->AttachedToDeviceObject,
&newDevExt->DeviceName) );
return STATUS_SUCCESS;
}

interval.QuadPart = (500 * DELAY_ONE_MILLISECOND); //delay 1/2 second
KeDelayExecutionThread( KernelMode, FALSE, &interval );
}

return status;
}



VOID SfCleanupMountedDevice( IN PDEVICE_OBJECT DeviceObject )
{

UNREFERENCED_PARAMETER( DeviceObject );
ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
}



VOID SfGetObjectName( IN PVOID Object, IN OUT PUNICODE_STRING Name )
{
NTSTATUS status;
CHAR nibuf[512]; //接收返回的对象名
POBJECT_NAME_INFORMATION nameInfo = (POBJECT_NAME_INFORMATION)nibuf; //nameInfo存放的是指向UNICODE_STRING结构变量的指针
ULONG retLength; //接收"实际返回的对象名长度"

status = ObQueryNameString( Object, nameInfo, sizeof(nibuf), &retLength); //返回对象名
Name->Length = 0;
if (NT_SUCCESS( status ))
{
RtlCopyUnicodeString( Name, &nameInfo->Name );
}
}



#if WINVER >= 0x0501

VOID SfGetBaseDeviceObjectName( IN PDEVICE_OBJECT DeviceObject, IN OUT PUNICODE_STRING Name )
{
ASSERT( NULL != gSfDynamicFunctions.GetDeviceAttachmentBaseRef );
DeviceObject = (gSfDynamicFunctions.GetDeviceAttachmentBaseRef)( DeviceObject );
SfGetObjectName( DeviceObject, Name );
ObDereferenceObject( DeviceObject );
}
#endif



PUNICODE_STRING SfGetFileName( IN PFILE_OBJECT FileObject, IN NTSTATUS CreateStatus, IN OUT PGET_NAME_CONTROL NameControl )
{
POBJECT_NAME_INFORMATION nameInfo;
NTSTATUS status;
ULONG size;
ULONG bufferSize;

NameControl->allocatedBuffer = NULL;
nameInfo = (POBJECT_NAME_INFORMATION)NameControl->smallBuffer;
bufferSize = sizeof(NameControl->smallBuffer);
status = ObQueryNameString( (NT_SUCCESS( CreateStatus ) ? (PVOID)FileObject : (PVOID)FileObject->DeviceObject), nameInfo, bufferSize, &size );
if (status == STATUS_BUFFER_OVERFLOW)
{
bufferSize = size + sizeof(WCHAR);
NameControl->allocatedBuffer =(PCHAR) ExAllocatePoolWithTag( NonPagedPool, bufferSize, SFLT_POOL_TAG );
if (NULL == NameControl->allocatedBuffer)
{
RtlInitEmptyUnicodeString(
(PUNICODE_STRING)&NameControl->smallBuffer,
(PWCHAR)(NameControl->smallBuffer + sizeof(UNICODE_STRING)),
(USHORT)(sizeof(NameControl->smallBuffer) - sizeof(UNICODE_STRING)) );

return (PUNICODE_STRING)&NameControl->smallBuffer;
}

nameInfo = (POBJECT_NAME_INFORMATION)NameControl->allocatedBuffer;
status = ObQueryNameString(
FileObject,
nameInfo,
bufferSize,
&size );
}

if (NT_SUCCESS( status ) && !NT_SUCCESS( CreateStatus ))
{
ULONG newSize;
PCHAR newBuffer;
POBJECT_NAME_INFORMATION newNameInfo;
newSize = size + FileObject->FileName.Length;
if (NULL != FileObject->RelatedFileObject)
{
newSize += FileObject->RelatedFileObject->FileName.Length + sizeof(WCHAR);
}

if (newSize > bufferSize)
{
newBuffer =(PCHAR) ExAllocatePoolWithTag( NonPagedPool, newSize, SFLT_POOL_TAG );
if (NULL == newBuffer)
{
RtlInitEmptyUnicodeString(
(PUNICODE_STRING)&NameControl->smallBuffer,
(PWCHAR)(NameControl->smallBuffer + sizeof(UNICODE_STRING)),
(USHORT)(sizeof(NameControl->smallBuffer) - sizeof(UNICODE_STRING)) );

return (PUNICODE_STRING)&NameControl->smallBuffer;
}

newNameInfo = (POBJECT_NAME_INFORMATION)newBuffer;

RtlInitEmptyUnicodeString(
&newNameInfo->Name,
(PWCHAR)(newBuffer + sizeof(OBJECT_NAME_INFORMATION)),
(USHORT)(newSize - sizeof(OBJECT_NAME_INFORMATION)) );

RtlCopyUnicodeString( &newNameInfo->Name, &nameInfo->Name );
if (NULL != NameControl->allocatedBuffer)
{
ExFreePool( NameControl->allocatedBuffer );
}

NameControl->allocatedBuffer = newBuffer;
bufferSize = newSize;
nameInfo = newNameInfo;
}
else
{
nameInfo->Name.MaximumLength = (USHORT)(bufferSize - sizeof(OBJECT_NAME_INFORMATION));
}

if (NULL != FileObject->RelatedFileObject)
{
RtlAppendUnicodeStringToString( &nameInfo->Name, &FileObject->RelatedFileObject->FileName );
RtlAppendUnicodeToString( &nameInfo->Name, L"\\" );
}

RtlAppendUnicodeStringToString( &nameInfo->Name, &FileObject->FileName );
ASSERT(nameInfo->Name.Length <= nameInfo->Name.MaximumLength);
}

return &nameInfo->Name;
}



VOID SfGetFileNameCleanup( IN OUT PGET_NAME_CONTROL NameControl )
{

if (NULL != NameControl->allocatedBuffer) {

ExFreePool( NameControl->allocatedBuffer);
NameControl->allocatedBuffer = NULL;
}
}



BOOLEAN SfIsAttachedToDevice( PDEVICE_OBJECT DeviceObject, PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL )
{

PAGED_CODE();

#if WINVER >= 0x0501
if (IS_WINDOWSXP_OR_LATER())
{
ASSERT( NULL != gSfDynamicFunctions.GetLowerDeviceObject && NULL != gSfDynamicFunctions.GetDeviceAttachmentBaseRef );
return SfIsAttachedToDeviceWXPAndLater( DeviceObject, AttachedDeviceObject );
}
else
{
#endif
return SfIsAttachedToDeviceW2K( DeviceObject, AttachedDeviceObject );
#if WINVER >= 0x0501
}
#endif
}



BOOLEAN SfIsAttachedToDeviceW2K( PDEVICE_OBJECT DeviceObject, PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL )
{
PDEVICE_OBJECT currentDevice;
PAGED_CODE();

for (currentDevice = DeviceObject; currentDevice != NULL; currentDevice = currentDevice->AttachedDevice)
{
if (IS_MY_DEVICE_OBJECT( currentDevice ))
{
if (ARGUMENT_PRESENT(AttachedDeviceObject))
{
ObReferenceObject( currentDevice );
*AttachedDeviceObject = currentDevice;
}
return TRUE;
}
}

if (ARGUMENT_PRESENT(AttachedDeviceObject))
{
*AttachedDeviceObject = NULL;
}
return FALSE;
}



#if WINVER >= 0x0501
BOOLEAN SfIsAttachedToDeviceWXPAndLater( PDEVICE_OBJECT DeviceObject, PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL )
{
PDEVICE_OBJECT currentDevObj;
PDEVICE_OBJECT nextDevObj;

PAGED_CODE();

ASSERT( NULL != gSfDynamicFunctions.GetAttachedDeviceReference );
currentDevObj = (gSfDynamicFunctions.GetAttachedDeviceReference)( DeviceObject );

do {

if (IS_MY_DEVICE_OBJECT( currentDevObj ))
{
if (ARGUMENT_PRESENT(AttachedDeviceObject))
{
*AttachedDeviceObject = currentDevObj;
}
else
{
ObDereferenceObject( currentDevObj );
}
return TRUE;
}

ASSERT( NULL != gSfDynamicFunctions.GetLowerDeviceObject );
nextDevObj = (gSfDynamicFunctions.GetLowerDeviceObject)( currentDevObj );
ObDereferenceObject( currentDevObj );
currentDevObj = nextDevObj;
} while (NULL != currentDevObj);

if (ARGUMENT_PRESENT(AttachedDeviceObject))
{
*AttachedDeviceObject = NULL;
}

return FALSE;
}
#endif



VOID SfReadDriverParameters( IN PUNICODE_STRING RegistryPath )
{
KdPrint(("RegistryPath:%s",RegistryPath));
OBJECT_ATTRIBUTES attributes;
HANDLE driverRegKey;
NTSTATUS status;
ULONG resultLength;
UNICODE_STRING valueName;
UCHAR buffer[sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + sizeof( LONG )];

PAGED_CODE();

if (0 == SfDebug)
{
InitializeObjectAttributes( &attributes,
RegistryPath,
OBJ_CASE_INSENSITIVE,
NULL,
NULL );

status = ZwOpenKey( &driverRegKey, KEY_READ, &attributes );
if (!NT_SUCCESS( status ))
{
	KdPrint(("read driver failed\n"));
return;
}

RtlInitUnicodeString( &valueName, L"DebugFlags" );
status = ZwQueryValueKey( driverRegKey,
&valueName,       
KeyValuePartialInformation,
buffer,
sizeof(buffer),
&resultLength );
if (NT_SUCCESS( status ))
{
SfDebug = *((PLONG) &(((PKEY_VALUE_PARTIAL_INFORMATION) buffer)->Data));
}

ZwClose(driverRegKey);
}
}




NTSTATUS SfIsShadowCopyVolume( IN PDEVICE_OBJECT StorageStackDeviceObject, OUT PBOOLEAN IsShadowCopy )
{

/*******************************************************************************************************************************************
SfIsShadowCopyVolume( )函数的主要功能是：涉及到 卷影拷贝服务的。

卷影拷贝服务(Volume Shadow Copy Service，VSS)是一种备份和恢复的技术。它是一种基于时间点来备份文件拷贝的技术。
通过使用卷影拷贝服务，我们可以在特定卷上建立数据拷贝时间点，并在将来的某一时刻把数据恢复到任何一个你曾创建的时间点的状态。
最常见的请求一般是恢复人为原因造成的数据丢失。用户不经意地存储了有错误信息的文件，或者是不小心删除文件，或是其他的数据意外。
备份VSS快照产生干净的数据镜像以及恢复时间点拷贝的能力。我们既可以恢复整个快照，也可以取所需，或者还可以使用VSS备份工具来恢复单独的文件和文件夹。

VSS不是对应用程序进行备份的，VSS可以对逻辑卷(硬盘分区)进行快照。
VSS是Windows下的快照技术，由Requestor, Write,和Provider组成；主要由驱动程序Volsnap.sys实现，被加到卷管理驱动和文件系统驱动之间，同时集成了COM技术。
因此，它不再是单单在卷级上的block进行处理，而是和各种系统应用相关联，比如SQL，EXCHANGE，AD等等。从而使得在不关机，也不停止应用的情况下，做快照。
VSS被广泛的应用到Windows的备份处理中。

VSS 解决问题的方法是，通过提供以下三个重要实体之间的通讯来保证备份的高度真实和恢复过程的简便。
(1)请求程序：它是用来请求时间点数据副本或卷影副本的应用程序，比如备份或存储管理应用程序。
(2)写入程序：它们负责数据通知和数据保护。写入程序是VSS区别于其它卷影副本或快照解决方案的地方。在VSS的卷影复制过程中会涉及一些应用程序。
(3)提供程序：它用于暴露基于硬件或软件的卷影副本的机制。许多存储硬件供应商都会为它们的存储阵列编写提供程序。


VSS服务唯一的缺点是：
我们需要为每一个卷影留出更多的磁盘空间，我们必须在某处存储这些拷贝。因为VSS使用指针数据，这些拷贝占用的空间要比想像的小得多，我们可以有效地存储这些拷贝。


有关VSS的更多说明，可以去浏览Microsoft的下述网站

http://technet.microsoft.com/en-us/library/ee923636.aspx

*********************************************************************************************************************************************/
PAGED_CODE();

*IsShadowCopy = FALSE;

#if WINVER >= 0x0501
if (IS_WINDOWS2000())
{
#endif
UNREFERENCED_PARAMETER( StorageStackDeviceObject );
return STATUS_SUCCESS;
#if WINVER >= 0x0501
}

if (IS_WINDOWSXP())
{
UNICODE_STRING volSnapDriverName;
WCHAR buffer[MAX_DEVNAME_LENGTH];
PUNICODE_STRING storageDriverName;
ULONG returnedLength;
NTSTATUS status;

if (FILE_DEVICE_DISK != StorageStackDeviceObject->DeviceType)
{
return STATUS_SUCCESS;
}

storageDriverName = (PUNICODE_STRING) buffer;
RtlInitEmptyUnicodeString( storageDriverName, (PWCHAR)Add2Ptr(storageDriverName, sizeof( UNICODE_STRING ) ), sizeof( buffer ) - sizeof( UNICODE_STRING ) );
status = ObQueryNameString( StorageStackDeviceObject, (POBJECT_NAME_INFORMATION)storageDriverName, storageDriverName->MaximumLength, &returnedLength );
if (!NT_SUCCESS( status ))
{
return status;
}

RtlInitUnicodeString( &volSnapDriverName, L"\\Driver\\VolSnap" );
if (RtlEqualUnicodeString( storageDriverName, &volSnapDriverName, TRUE ))
{
*IsShadowCopy = TRUE;
}
else
{
NOTHING;
}

return STATUS_SUCCESS;
}
else
{
PIRP irp;
KEVENT event;
IO_STATUS_BLOCK iosb;
NTSTATUS status;
if (FILE_DEVICE_VIRTUAL_DISK != StorageStackDeviceObject->DeviceType)
{
return STATUS_SUCCESS;
}

KeInitializeEvent( &event, NotificationEvent, FALSE );

/*
*Microsoft WDK官方文档对 IOCTL_DISK_IS_WRITABLE是这样解释的：
*Determines whether a disk is writable.
*The Status field can be set to STATUS_SUCCESS, or possibly to STATUS_INSUFFICIENT_RESOURCES, STATUS_IO_DEVICE_ERROR, or STATUS_MEDIA_WRITE_PROTECTED.
*
*IOCTL_DISK_IS_WRITABLE是没有输入也没有输出的。
*/
irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_IS_WRITABLE,
StorageStackDeviceObject,
NULL,
0,
NULL,
0,
FALSE,
&event,
&iosb );
if (irp == NULL)
{
return STATUS_INSUFFICIENT_RESOURCES;
}

status = IoCallDriver( StorageStackDeviceObject, irp );
if (status == STATUS_PENDING)
{
(VOID)KeWaitForSingleObject( &event,
Executive,
KernelMode,
FALSE,
NULL );
status = iosb.Status;
}

if (STATUS_MEDIA_WRITE_PROTECTED == status)
{
*IsShadowCopy = TRUE;
status = STATUS_SUCCESS;
}

return status;
}
#endif
}
//用于捕获读操作的分发例程
NTSTATUS SfRead(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp)
{
//	KdPrint(("SfReaad in2!\n"));
	bool MyDebug=MY_DEBUG(MYDEBUG,READ_DEBUG);
	
    NTSTATUS statue;
	PIO_STACK_LOCATION irpsp=IoGetCurrentIrpStackLocation(Irp);
	PFILE_OBJECT file_object=irpsp->FileObject;
	PSFILTER_DEVICE_EXTENSION devExt=(PSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
	if(IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject))
	{
		KdPrint(("1\n"));
		Irp->IoStatus.Status=STATUS_INVALID_DEVICE_REQUEST;
		Irp->IoStatus.Information=0;
		IoCompleteRequest(Irp,IO_NO_INCREMENT);
		return STATUS_INVALID_DEVICE_REQUEST;
	}
	if(devExt->StorageStackDeviceObject!=NULL)//这是对卷设备的操作，不是文件
	{
		
		//return SfPassThrough(DeviceObject,Irp);
	    
		//KdPrint(("2.17\n"));
	    KEVENT event;
	    PUCHAR buf;
		ULONG length;
		LARGE_INTEGER offset;
		UCHAR mini_code=irpsp->MinorFunction;
		::KeInitializeEvent(&event,NotificationEvent,FALSE);
		::IoCopyCurrentIrpStackLocationToNext(Irp);
		::IoSetCompletionRoutine(Irp,
			                     SfReadCompleteion,
								 &event,
								 true,
								 true,
								 true);
	//	if(MyDebug)
	//	KdPrint(("IRP_MJ_READ进行\n"));
		statue=::IoCallDriver(devExt->AttachedToDeviceObject,Irp);
		if(statue==STATUS_PENDING)
		{
			statue=KeWaitForSingleObject(&event,Executive,KernelMode,FALSE,NULL);
		//	ASSERT(statue!=STATUS_SUCCESS);
		}
	if(Irp->IoStatus.Status==STATUS_SUCCESS)
	{
		irpsp=IoGetCurrentIrpStackLocation(Irp);
		offset.QuadPart=irpsp->Parameters.Read.ByteOffset.QuadPart;

	   length=irpsp->Parameters.Read.Length;
	    UNICODE_STRING tocompare;
		
		::RtlInitUnicodeString(&tocompare,(PCWSTR)L"\\fortest\\90\\devicetree.exe");
	//	KdPrint(("filename:%wZ ",&(irpsp->FileObject->FileName)));
	   if(RtlCompareUnicodeString(&(irpsp->FileObject->FileName),&tocompare,true)!=0)
	   {
		  // if(MyDebug)
		 //  KdPrint(("passthrough,filename:%wZ ",&(irpsp->FileObject->FileName)));
		   return SfPassThrough(DeviceObject,Irp);
	   }
	   else
		   KdPrint(("加密filename:%wZ ",&(irpsp->FileObject->FileName)));

		switch(mini_code)
			{
		case(IRP_MN_NORMAL):
		{
			if(Irp->MdlAddress!=NULL)
			{
				KdPrint(("MDLAddress \n"));
				buf=(PUCHAR)MmGetSystemAddressForMdlSafe(Irp->MdlAddress,NormalPagePriority);
				if(buf==NULL)
				{
					KdPrint(("there are a error occured in MmGetSystemAddressForMdl!\n"));
					buf=(PUCHAR)Irp->UserBuffer;
				}
			}
			else
				buf=(PUCHAR)Irp->UserBuffer;
			for(int i=0;i<length;++i)
				buf[i]^=0x77;
			//KdPrint(("NORMAL:读取的内容1:%wZ",buf));
			if(MyDebug)
			KdPrint(("content:%c%c%c%c",buf[0],buf[1],buf[2],buf[3]));
			//RtlFillMemory(buf,length,'U');
			Irp->IoStatus.Information=length;
			Irp->IoStatus.Status=STATUS_SUCCESS;			
			irpsp->FileObject->CurrentByteOffset.QuadPart=offset.QuadPart+length;
			
		    IoCompleteRequest(Irp,IO_NO_INCREMENT);
			return STATUS_SUCCESS;
		}
		case(IRP_MN_MDL):
			//PAGE_SIZE * (65535 - sizeof(MDL)) / sizeof(ULONG_PTR)这是winodows7以前的最大的可分配的mdl所描述的内存的size;
		{
			PMDL mdl=::MyMdlMemoryAllocate(length);
			if(mdl==NULL)
			{
				return STATUS_INSUFFICIENT_RESOURCES;
			}
			Irp->MdlAddress=mdl;
		    buf=(PUCHAR)MmGetSystemAddressForMdlSafe(mdl,NormalPagePriority);
			//KdPrint(("MN_MDL读取的内容2:%wZ",buf));
			Irp->IoStatus.Information=length;
			Irp->IoStatus.Status=STATUS_SUCCESS;
			irpsp->FileObject->CurrentByteOffset.QuadPart=offset.QuadPart+length;
			IoCompleteRequest(Irp,IO_NO_INCREMENT);
			return STATUS_SUCCESS;

		}
		case(IRP_MN_COMPLETE_MDL):
		{
			::MyFreeMdl(Irp->MdlAddress);
			Irp->IoStatus.Information=length;
			Irp->IoStatus.Status=STATUS_SUCCESS;
			irpsp->FileObject->CurrentByteOffset.QuadPart=offset.QuadPart+length;
			IoCompleteRequest(Irp,IO_NO_INCREMENT);
			return STATUS_SUCCESS;
		}
		default:
			{
				KdPrint(("其他类型的IRP_MJ_READ\n"));
				return STATUS_INVALID_DEVICE_REQUEST;
			}
	}
		


   }
	}
	
  // KdPrint(("4\n"));
   statue=STATUS_SUCCESS;
	return statue;
}
_inline PMDL MyMdlAllocate(PVOID buff,ULONG length)
{
	PMDL mdl=::IoAllocateMdl(buff,length,false,false,NULL);
	if(mdl==NULL)
		return NULL;
	::MmBuildMdlForNonPagedPool(mdl);
	return mdl;
}
_inline PMDL MyMdlMemoryAllocate(ULONG length)
{
	PVOID buf=::ExAllocatePool(NonPagedPool,length);
	if(buf==NULL)
		return NULL;
	PMDL mdl=::MyMdlAllocate(buf,length);
	if(mdl==NULL)
	{
		::ExFreePool(buf);
		return NULL;
	}
	return mdl;
}
_inline VOID MyFreeMdl(PMDL mdl)
{
	void *buf=MmGetSystemAddressForMdlSafe(mdl,NormalPagePriority);
	::IoFreeMdl(mdl);
	::ExFreePool(buf);
}
NTSTATUS SfReadCompleteion(PDEVICE_OBJECT pdevice,PIRP irp,PVOID Context)
{
	PKEVENT waitevent=(PKEVENT)Context;
	
	UNREFERENCED_PARAMETER( pdevice );

	if(irp->IoStatus.Status==STATUS_SUCCESS&&(MY_DEBUG(MYDEBUG,READ_DEBUG)))
	{
	//	KdPrint(("SfReadCompleteion finised 1.1!\n"));
	}
	KeSetEvent(waitevent,IO_NO_INCREMENT,FALSE);
	ASSERT(IS_MY_DEVICE_OBJECT(pdevice));

	return STATUS_MORE_PROCESSING_REQUIRED;
}
BOOLEAN LookForDirectory(PFILE_OBJECT fileobject)
{
	PLIST_ENTRY p=NULL;
	for(p=listentry.Flink;p!=(PLIST_ENTRY)&listentry;p=p->Blink)
	{
		
		PAll_DIRECTORS pmd=CONTAINING_RECORD(p,All_DIRECTORS,link);
	//	KdPrint(("number:%ld",pmd->directors_number));
		if(fileobject==pmd->FileObj)
		{
		//	KdPrint(("在目录集合中找到该目录:%l",pmd->directors_number));
			return true;
		}
	}
	return false;
}

// <=DISPATCH

/*PUNICODE_STRING MyFileFullPathQueryW(IN PFILE_OBJECT file,ULONG size)
{
	NTSTATUS status;
	POBJECT_NAME_INFORMATION nameinfo=NULL;
	UNICODE_STRING path;
	path.Buffer=(PWCHAR)::ExAllocatePool(NonPagedPool,size);
	path.MaximumLength=size;
	path.Length=0;
	RtlZeroMemory(&path,size);

	WCHAR buf[FILE_NAME_LENGTH_MAX]={0};
    void *obj_ptr;
	ULONG length=0;
	BOOLEAN need_split=FALSE;
	ASSERT(file!=NULL);
	if(file==NULL)
	{
		return 0;
	}
	if(file->FileName.Buffer==NULL)
		return 0;
	nameinfo=(POBJECT_NAME_INFORMATION)buf;
	if(file->RelatedFileObject!=NULL)
		obj_ptr=(void*)file->RelatedFileObject;
	else
		obj_ptr=(void*)file->DeviceObject;
	status=ObQueryNameString(obj_ptr,nameinfo,64*sizeof(WCHAR),&length);
	do
	{
	
	if(status==STATUS_INFO_LENGTH_MISMATCH)
	{
		nameinfo=(POBJECT_NAME_INFORMATION )ExAllocatePoolWithTag(NonPagedPool,length,MEM_TAG);
		if(nameinfo==NULL)
			return NULL;
		RtlZeroMemory(nameinfo,length);
		status=ObQueryNameString(obj_ptr,nameinfo,length,&length);
	}
	if(!NT_SUCCESS(status))
	{
		KdPrint(("ObQueryNameSreing failed\n"));
		break;
	}
	if(file->FileName.Length>2&&file->FileName.Buffer[0]!=L'\\'&&nameinfo->Name.Buffer[nameinfo->Name.Length/sizeof(WCHAR)-1]!=L'\\')
		need_split=TRUE;
	length=nameinfo->Name.Length+file->FileName.Length;
	if(need_split)
		length+=sizeof(WCHAR);
	if(path.MaximumLength<length)
	{
		KdPrint(("length is not enough :%d",path.MaximumLength));
		
		break;
	}
	::RtlCopyUnicodeString(&path,&nameinfo->Name);
	//KdPrint(("devicenameinfo:%wZ",nameinfo->Name));
	if(need_split)
		RtlAppendUnicodeStringToString(&path,&file->FileName);
	//KdPrint(("filename:%wZ",file->FileName));
	}while(0);
	if((void*)nameinfo!=(void*)buf)
		  ExFreePool(nameinfo);
	return &path;
}*/

NTSTATUS SfDirectoryControl(IN PDEVICE_OBJECT device,IN PIRP irp)
{
	NTSTATUS statu;
	UNREFERENCED_PARAMETER(device);
	PIO_STACK_LOCATION  irpsp=::IoGetCurrentIrpStackLocation(irp);
	KIRQL oldirql=::KeGetCurrentIrql();

	POperationInformation operationinformation;
	POperationResult result;

	
	
	PUNICODE_STRING directory=NULL;
	KdPrint(("目录访问控制\n"));
	statu=DBAC_ControlLogicEx(directory,IRP_MJ_SET_INFORMATION,irpsp->FileObject);
	if(statu!=0x1000)  //由默认控制逻辑返回了状态，空则代表需要app验证,初始为返回通过
	{	   
		KdPrint(("statu:%x",statu));
		   goto FREE;
	}

	
	if(oldirql>PASSIVE_LEVEL)
	{
		KdPrint(("High Irql level in SfDirectoryControl!\n"));
	}
	else
	{
	
		    KdPrint(("statu==NULL: %d",statu==NULL?1:0));
		    ::AllocatePool(operationinformation,result);
			::RtlInitUnicodeString(&(operationinformation->FileName),(PCWSTR)directory->Buffer);

			::RtlInitUnicodeString(&(operationinformation->FilePath),(PCWSTR)directory->Buffer);
			operationinformation->OperationType=IRP_MJ_SET_INFORMATION;
			result->operationtype=IRP_MJ_SET_INFORMATION;
			statu=::VerifyControl(operationinformation,result);
			
			goto FREE;
		

	}
    goto FREE;

	FREE:
	if(NT_SUCCESS(statu))//如果通过则下发到下面的驱动
	{
	//::ExFreePool(filename);
	::IoSkipCurrentIrpStackLocation(irp);
	return IoCallDriver( ((PSFILTER_DEVICE_EXTENSION)device->DeviceExtension)->AttachedToDeviceObject, irp );

	}
//	::ExFreePool(filename);
	irp->IoStatus.Information=0;
	irp->IoStatus.Status=statu;
	IoCompleteRequest(irp,IO_NO_INCREMENT);
	return statu;
}

NTSTATUS SfSetInformation(IN PDEVICE_OBJECT device,IN PIRP irp)
{


	NTSTATUS statu;
	UNREFERENCED_PARAMETER(device);
	PIO_STACK_LOCATION  irpsp=::IoGetCurrentIrpStackLocation(irp);
	POperationInformation operationinformation;
	POperationResult result;
	
	
	PUNICODE_STRING directory=NULL;

	statu=DBAC_ControlLogicEx(directory,IRP_MJ_SET_INFORMATION,irpsp->FileObject);
	if(statu!=0x1000)  //由默认控制逻辑返回了状态，空则代表需要app验证,初始为返回通过
	{	   
		KdPrint(("statu:%x",statu));
		   goto FREE;
	}
	

	if(irpsp->Parameters.SetFile.FileInformationClass==FileDispositionInformation ||irpsp->Parameters.SetFile.FileInformationClass==FileRenameInformation )
	{			
		    KdPrint(("statu==NULL: %d",statu==NULL?1:0));
		    ::AllocatePool(operationinformation,result);
			::RtlInitUnicodeString(&(operationinformation->FileName),(PCWSTR)directory->Buffer);

			::RtlInitUnicodeString(&(operationinformation->FilePath),(PCWSTR)directory->Buffer);
			operationinformation->OperationType=IRP_MJ_SET_INFORMATION;
			result->operationtype=IRP_MJ_SET_INFORMATION;
			statu=::VerifyControl(operationinformation,result);
			
			goto FREE;

	}

   goto FREE;

FREE:
	if(NT_SUCCESS(statu))//如果通过则下发到下面的驱动
	{
	
	::IoSkipCurrentIrpStackLocation(irp);
	return IoCallDriver( ((PSFILTER_DEVICE_EXTENSION)device->DeviceExtension)->AttachedToDeviceObject, irp );

	}
	
	irp->IoStatus.Information=0;
	irp->IoStatus.Status=statu;
	IoCompleteRequest(irp,IO_NO_INCREMENT);
	return statu;
}


//和应用通信的IRP派遣函数
NTSTATUS SfDeviceControl(IN PDEVICE_OBJECT device,IN PIRP irp)
{
	
	if(device==gSFilterControlDeviceObject)
	{
	KdPrint(("device io control IN!\n"));
	
	NTSTATUS status=STATUS_SUCCESS;
	PIO_STACK_LOCATION irpsp=::IoGetCurrentIrpStackLocation(irp);
	ULONG iocode=irpsp->Parameters.DeviceIoControl.IoControlCode;
	
	switch(iocode)
	{
	case(IOCTL_DEVICE_READ):
		break;
	case(IOCTL_DEVICE_WRITE):

		break;
	case(IOCTL_CAREFILE_INIT):
		KdPrint(("收到询问:是否需要初始化!\n"));
		CR_SetCareFileInit(irp);
		break;
	case(IOCTL_CAREFILE_START_INIT):
		KdPrint(("开始传送文件数据，并且初始化关心文件链表!\n\n"));
		CR_StartInitCareFileHashTable(irp);
		break;
	case(IOCTL_CAREFILE_INIT_SHUTDOWN):
		KdPrint(("收到询问:初始化是否正确完成!\n"));
		CR_InitFinished(irp);
		break;
	case(IOCTL_VERIFY_OPERATION_INFORMATION)://
	    KdPrint(("收到了来自应用的取操作信息通知！call GetOperationInformation(irp)\n"));
		GetOperationInformation(irp);
		//DisPlayHashTable(1);
		break;
	case(IOCTL_SEND_VERIFY_RESULT):
		KdPrint(("收到了来自应用程序的操作验证结果通知!\n"));		
		::SetOperationResult(irp);
	//	DisPlayHashTable(2);
		break;
	default:
		KdPrint(("UnKown Device Control Code:%d!",iocode));
		break;
	}
	
	KdPrint(("SfDeviceControl  finished!\n\n"));
	irp->IoStatus.Information=0;
	irp->IoStatus.Status=STATUS_SUCCESS;
	IoCompleteRequest(irp,IO_NO_INCREMENT);	
	return STATUS_SUCCESS;
	}
	else
	{
		KdPrint(("device io control: call nextdeivce\n"));
		::IoSkipCurrentIrpStackLocation(irp);
		return IoCallDriver( ((PSFILTER_DEVICE_EXTENSION)device->DeviceExtension)->AttachedToDeviceObject, irp );
	}
	    

}



