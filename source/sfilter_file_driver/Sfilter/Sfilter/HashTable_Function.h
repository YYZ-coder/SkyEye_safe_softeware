

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifdef __cplusplus
extern "C" 
{

#endif
#define CAREFILETABLE_LENGTH 30
#define FILE_NAME_LENGTH_MAX 512
#define FILE_PATH_LENGTH_MAX 1024

#define HASH_TABLE_NODE_TAG 'node'
#define HASH_TABLE_LENGTH 30
#define MEM_TAG 'mymt'
#define MEM_HASH_TAG 'hash'
#define MEM_CAREFILE_NODE 'care'
#define READ_DEBUG 0x0001
#define DIRECTORY_DEBUG 0x0010
#define LOOKASIDE_LIST_FOR_OPERATION   '1234'
#define LOOKASIDE_LIST_FOR_RESULT      '3456'
#define LOOKASIDE_LIST_FOR_CAREFILE    '4567'
#define LOOKASIDE_LIST_FOR_CAREFILE_VERIFY   '4678'
#define EACH_NUMBER                     5
#define FILE_UNKOWN FILE_DEVICE_DISK_FILE_SYSTEM
#define IOCTL_DEVICE_READ CTL_CODE(FILE_UNKOWN,0x8089,METHOD_IN_DIRECT,FILE_ANY_ACCESS)
#define IOCTL_DEVICE_WRITE CTL_CODE(FILE_UNKOWN,0x807,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)
#define IOCTL_DEVICE_PRINT CTL_CODE(FILE_UNKOWN,0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_QUERY_REGISTRY_KEY CTL_CODE(FILE_UNKOWN,0x811,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

#define IOCTL_VERIFY_OPERATION_INFORMATION CTL_CODE(FILE_UNKOWN,0x812,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)//用于从内核中取出需要验证的操作信息
#define IOCTL_SEND_VERIFY_RESULT CTL_CODE(FILE_UNKOWN,0x813,METHOD_IN_DIRECT,FILE_ANY_ACCESS) //用于返回验证的信息
#define IOCTL_EXCEPTION_BUFFER_LENGTH_NOT_ENOUGH CTL_CODE(FILE_UNKOWN,0X814,METHOD_IN_DIRECT,FILE_ANY_ACCESS)//意外的情况，缓冲区的长度不够，请求分配更多的内存

#define IOCTL_CAREFILE_INIT       CTL_CODE(FILE_UNKOWN,0x815,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)//用于初始化关心文件链表
#define IOCTL_CAREFILE_START_INIT  CTL_CODE(FILE_UNKOWN,0x816,METHOD_IN_DIRECT,FILE_ANY_ACCESS)//开始初始化
#define IOCTL_CAREFILE_INIT_SHUTDOWN   CTL_CODE(FILE_UNKOWN,0x817,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)//内核已经初始化完毕

#define FILE_READ 0x1
#define FILE_MODIFY 0x2
#define FILE_REMOVE 0x3
#define FILE_CREATES 0x4
#define FILE_WRITE 0x5

#define EXCEPTION_NONE_RECORD 0x001           //没有记录
#define EXCEPTION_APPLICATION_ERROR 0x002     //内部程序错误，信息记录在message里
//**********************************************
//--------------------------------------------------------------------------------------------
//这个结构用于存储所有被认为是目录的文件对象
#define HASH_TABLE_LENGTH 30
typedef struct _HASH_TABLE_NODE{
	LIST_ENTRY entry;
	ULONG number; //当前的node下面挂接了几个节点
	ULONG totalnumbers;
	ULONG NodeID; //当前节点的ID，可以简单的设置为在hash表里的序号
	ULONG OperationType;//当前节点所关心的操作类型，这些类型会都挂接在当前节点下面
	KSPIN_LOCK node_lock;
	KIRQL node_irql;
	bool initilzed;
}HASH_TABLE_NODE,*PHASH_TABLE_NODE;

typedef struct _HASH_TABLE_NODE_CAREFILE
{
	LIST_ENTRY entry;
	ULONG number;
	ULONG totalnumbers;
	ULONG NodeID;
	ULONG firstchar;
	KSPIN_LOCK node_lock;
	KIRQL node_irql;
	bool initilzed;

}HASH_TABLE_CAREFILE_NODE,*PHASH_TABLE_CAREFILE_NODE;
typedef struct _HASH_TABLE_CAREFILE
{
	HASH_TABLE_CAREFILE_NODE link[CAREFILETABLE_LENGTH];
	unsigned short Length;
}HASH_TABLE_CAREFILE,*PHASH_TABLE_CAREFILE;
typedef struct _HASH_TABLE{
	HASH_TABLE_NODE link[HASH_TABLE_LENGTH];
	unsigned short Length;
}HASH_TABLE,*PHASH_TABLE; 



 typedef struct _All_Directors{
	 LIST_ENTRY link;
	 ULONG directors_number;
	 PFILE_OBJECT FileObj;
	 UNICODE_STRING filename;

 }All_DIRECTORS,*PAll_DIRECTORS;

VOID MyAddToLinkSet(PFILE_OBJECT fileobject,PKSPIN_LOCK kspn);
BOOLEAN LookForDirectory(PFILE_OBJECT fileobject);
ULONG MyFileFullPathQuery(PFILE_OBJECT file,PUNICODE_STRING path);
PUNICODE_STRING MyFileFullPathQueryW(IN PFILE_OBJECT file,ULONG size);


_inline PMDL MyMdlMemoryAllocate(ULONG length);
_inline VOID MyFreeMdl(PMDL mdl);



//********************************存储操作信息************************************
typedef struct _OperationInformation{
	LIST_ENTRY listentry;
	ULONG OperationID;  //这个标志着当前结构变量的ID
	ULONG OperationType;
	UNICODE_STRING FileName;
	UNICODE_STRING FilePath;
	KEVENT finished;           //用于在操作被验证并且返回的时候设置
	bool wasdealwith;          //标志这个操作信息是否正在被处理
}OperationInformation,*POperationInformation;

//***************************专用于存储和操作类型对应的线程全局事件************

typedef struct _ThreadEvents{
	KIRQL oldirql;
	KSPIN_LOCK nodelock;
	PKEVENT  event;
	ULONG OperationType;
	BOOLEAN waitstatue; //保存当前是否为等待的状态
}ThreadEvents,*PThreadEvents;
//*************************用于交互的操作信息结构*********************************

 typedef struct _operationinrecord   //传递给用户程序并且等待其返回
{
	ULONG OperationID;    
	BOOLEAN lock;              //文件是否加锁
	HANDLE event;           //event object
	HANDLE forback;         //这是用于在内核下面等待用户程序的处理结果的事件
	ULONG  operationtype;
	BOOLEAN operation_access;  //操作是否允许
	WCHAR result[512];       //结果的详细的信息	
	WCHAR filepath[FILE_PATH_LENGTH_MAX];     //文件的路径
	WCHAR filename[FILE_NAME_LENGTH_MAX];     //文件名
	WCHAR otherinfo[512];    //其他信息
	BOOLEAN   exception;        //操作是否出现意外
	ULONG  exceptiontype;
	WCHAR  exceptionmessage[128]; //意外的信息
}OperationRecord,*POperationRecord;  //操作记录结构，这是用于传递给App的结构，也是在应用程序下进行判断的主要依据

typedef struct _operationresult  //记录操作结果，并返回给内核
{
	LIST_ENTRY entry;
	ULONG operationID;
	bool operation_permit;
	WCHAR otherinfo[512];
	ULONG operationtype; 
	bool  wasFill;   //是否被填写了
}OperationResult,*POperationResult;


//*********关心文件的结构**********
typedef struct _CareFile
{
	LIST_ENTRY entry;
	UNICODE_STRING filename;
	UNICODE_STRING filepath;
    INT            secutiry_level;
	ULONG          Owner;
}CareFile,*PCareFile;
//***************专用于传输的结构********
typedef struct _CareFile_T
{
	WCHAR filename[FILE_NAME_LENGTH_MAX];
	WCHAR filepath[FILE_PATH_LENGTH_MAX];
    INT            secutiry_level;
	ULONG          Owner;
}CareFile_T,*PCareFile_T;
typedef struct _CareFileTransfer
{
	ULONG    number;   //记录当前结构有多少CareFile结构
	CareFile_T filenode[EACH_NUMBER]; //数组的首地址
}CareFileTransfer,*PCareFileTransfer;
//***************默认的控制逻辑*********
typedef struct _DefaultControl
{

}DefaultControl,*PDefaultControl;

//**************用户信息****************
typedef struct _UserInfo
{
	INT UserID;
	INT Security_Level;

}UserInfo,*PUserInfo;
//**********************************************文件哈希表的操作******************************
VOID InitHashTable_CareFile();
NTSTATUS CR_SetCareFileInit(PIRP irp);
NTSTATUS CR_StartInitCareFileHashTable(PIRP irp);
NTSTATUS CR_InitFinished(PIRP irp);
VOID     CR_MyNodeLock(PHASH_TABLE_CAREFILE_NODE node);
VOID     CR_MyNodeUnLock(PHASH_TABLE_CAREFILE_NODE node);
//*********************************************哈希表的操作函数*********************************************

ULONG FillOperationInformationAndAddToList(ULONG IrpType,POperationInformation operationinformation,PVOID pContext);
VOID HashTableNodeListLock(PHASH_TABLE_NODE node);
VOID HashTableNodeListUnLock(PHASH_TABLE_NODE node);
ULONG MyOperationInformationAddToHashTable(POperationInformation operation);
VOID InitOperationInformation(POperationInformation infor);
VOID GetOperationInformation(PIRP irp);
unsigned short HashFunction_ForOperation(ULONG irptype);  //简单的返回
ULONG QueryListNodeCount(PLIST_ENTRY px);
ULONG GetOperationInformationFromHashTable(ULONG OperationType,OUT POperationInformation *operinfos);
PLIST_ENTRY GetListTailNode(PLIST_ENTRY px);
VOID GetOperationResult(POperationResult result,ULONG type,ULONG ID);
VOID SetOperationResult(PIRP irp);
VOID MyOperationResultAddToHashTable2(POperationResult result);
NTSTATUS FastIO_GetOperationInformation(ULONG iocontrolcode,PVOID inbuffer,PVOID outbuffer,ULONG inlen,ULONG outlen);
NTSTATUS FastIO_SetOperationResult(ULONG iocontrolcode,PVOID inbuffer,PVOID outbuffer,ULONG inlen,ULONG outlen);
VOID DisPlayHashTable(ULONG whichtable);
VOID PlayCurrentThreadName();
BOOLEAN FillOperationResultAddToHashTable_Result(POperationResult operationresult);
VOID  InitOperationResult(POperationResult re);
//*************************************
VOID KeMyEventsLock(ULONG number);
VOID KeMyEventsUnLock(ULONG number);
VOID HashTableNodeListLock(PHASH_TABLE_NODE node);
VOID HashTableNodeListUnLock(PHASH_TABLE_NODE node);
unsigned short HashFunction_ForOperation(ULONG irptype);
BOOLEAN IsCreateNewFile(PIRP irp);

ULONG QueryListNodeCount(PLIST_ENTRY px);
ULONG GetOperationInformationFromHashTable(ULONG OperationType,OUT POperationInformation *operinfos);
PLIST_ENTRY GetListTailNode(PLIST_ENTRY px);
VOID GetOperationResult(POperationResult result,ULONG type,ULONG ID);
VOID SetOperationResult(PIRP irp);
VOID MyOperationResultAddToHashTable2(POperationResult result);
NTSTATUS FastIO_GetOperationInformation(ULONG iocontrolcode,PVOID inbuffer,PVOID outbuffer,ULONG inlen,ULONG outlen);
NTSTATUS FastIO_SetOperationResult(ULONG iocontrolcode,PVOID inbuffer,PVOID outbuffer,ULONG inlen,ULONG outlen);
VOID DisPlayHashTable(ULONG whichtable);


VOID ProcessOffsetInit();
VOID InitHashTable(PHASH_TABLE a);
VOID MyAddToHashTable(PFILE_OBJECT fileobject,PKSPIN_LOCK spn,PUNICODE_STRING filename);
static const PCHAR DeviceTypeNames[] = {
"",
"BEEP",
"CD_ROM",
"CD_ROM_FILE_SYSTEM",
"CONTROLLER",
"DATALINK",
"DFS",
"DISK",
"DISK_FILE_SYSTEM",
"FILE_SYSTEM",
"INPORT_PORT",
"KEYBOARD",
"MAILSLOT",
"MIDI_IN",
"MIDI_OUT",
"MOUSE",
"MULTI_UNC_PROVIDER",
"NAMED_PIPE",
"NETWORK",
"NETWORK_BROWSER",
"NETWORK_FILE_SYSTEM",
"NULL",
"PARALLEL_PORT",
"PHYSICAL_NETCARD",
"PRINTER",
"SCANNER",
"SERIAL_MOUSE_PORT",
"SERIAL_PORT",
"SCREEN",
"SOUND",
"STREAMS",
"TAPE",
"TAPE_FILE_SYSTEM",
"TRANSPORT",
"UNKNOWN",
"VIDEO",
"VIRTUAL_DISK",
"WAVE_IN",
"WAVE_OUT",
"8042_PORT",
"NETWORK_REDIRECTOR",
"BATTERY",
"BUS_EXTENDER",
"MODEM",
"VDM",
"MASS_STORAGE",
"SMB",
"KS",
"CHANGER",
"SMARTCARD",
"ACPI",
"DVD",
"FULLSCREEN_VIDEO",
"DFS_FILE_SYSTEM",
"DFS_VOLUME",
"SERENUM",
"TERMSRV",
"KSEC"
};




#ifdef __cplusplus
}
#endif
