




#define FILE_NAME_LENGTH_MAX 512
#define FILE_PATH_LENGTH_MAX 1024

#define FILE_UNKOWN FILE_DEVICE_DISK_FILE_SYSTEM
#define IOCTL_DEVICE_WRITE CTL_CODE(FILE_UNKOWN,0x808,METHOD_IN_DIRECT,FILE_ANY_ACCESS)
#define IOCTL_DEVICE_READ CTL_CODE(FILE_UNKOWN,0x807,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)
#define IOCTL_DEVICE_PRINT CTL_CODE(FILE_UNKOWN,0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)
//#define IOCTL_GET_OPERATION_INFORMATION CTL_CODE(FILE_UNKOWN,0x810,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)
#define IOCTL_QUERY_REGISTRY_KEY CTL_CODE(FILE_UNKOWN,0x811,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

#define IOCTL_VERIFY_OPERATION_INFORMATION CTL_CODE(FILE_UNKOWN,0x812,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)//用于从内核中取出需要验证的操作信息
#define IOCTL_SEND_VERIFY_RESULT CTL_CODE(FILE_UNKOWN,0x813,METHOD_IN_DIRECT,FILE_ANY_ACCESS) //用于返回验证的信息
#define IOCTL_EXCEPTION_BUFFER_LENGTH_NOT_ENOUGH CTL_CODE(FILE_UNKOWN,0X814,METHOD_IN_DIRECT,FILE_ANY_ACCESS)//意外的情况，缓冲区的长度不够，请求分配更多的内存

#define IOCTL_CAREFILE_INIT  CTL_CODE(FILE_UNKOWN,0x815,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)//用于初始化关心文件链表
#define IOCTL_CAREFILE_START_INIT  CTL_CODE(FILE_UNKOWN,0x816,METHOD_IN_DIRECT,FILE_ANY_ACCESS)//开始初始化
#define IOCTL_CAREFILE_INIT_SHUTDOWN   CTL_CODE(FILE_UNKOWN,0x817,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)//内核已经初始化完毕

#define FILE_READ 0x1
#define FILE_MODIFY 0x2
#define FILE_REMOVE 0x3                                                                                                       
#define FILE_CREATE 0x4
#define FILE_WRITE 0x5

#define EXCEPTION_NONE_RECORD 0x001           //没有记录
#define EXCEPTION_APPLICATION_ERROR 0x002     //内部程序错误，信息记录在message里
#define EACH_NUMBER 5                       //CareFileTransfer传送结构有多少CareFileNode结构

typedef struct _operationinrecord   //传递给用户程序并且等待其返回
{
	ULONG OperationID;    
	BOOLEAN lock;              //文件是否加锁
//	int length;             //文件的长度
	HANDLE event;           //event object，紧急事件对象，出现意外的时候使用
	HANDLE forback;         //这是用于在内核下面等待用户程序的处理结果的事件
	ULONG  operationtype;
	BOOLEAN operation_access;  //操作是否允许
	WCHAR result[512];       //结果的详细的信息
	WCHAR filename[FILE_NAME_LENGTH_MAX];     //文件名
	WCHAR filepath[FILE_PATH_LENGTH_MAX];     //文件的路径
	WCHAR otherinfo[512];    //其他信息
	BOOLEAN   exception;        //操作是否出现意外
	ULONG  exceptiontype;
	WCHAR  exceptionmessage[128]; //意外的信息
}OperationRecord,*POperationRecord;  //操作记录结构，这是用于传递给App的结构，也是在应用程序下进行判断的主要依据

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
typedef struct _operationresult  //记录操作结果，并返回给内核
{
	LIST_ENTRY entry;
	ULONG operationID;
	bool operation_permit;
	WCHAR otherinfo[512];
	ULONG operationtype; 
	bool  wasFill;   //是否被填写了
}OperationResult,*POperationResult;



// TODO: reference additional headers your program requires here