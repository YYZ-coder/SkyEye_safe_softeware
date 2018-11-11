
#define MAX_FILE_PATH_LENGTH   1024
#define Name_Value_Number 3         //值对的最大数量:filename,filepath,security_level

#define MATCHING_ERROR_TYPE_ONE CAREFILETABLE_LENGTH-1  //匹配出现的错误类型
#define MATCHING_ERROR_TYPE_TWO CAREFILETABLE_LENGTH-2

#define NAME_VALUE_TYPE_FILENAME        0x1        //名的类型
#define NAME_VALUE_TYPE_FILEPATH        0x2
#define NAME_VALUE_TYPE_OWNER           0x3
#define NAME_VALUE_TYPE_SECURITY_LEVEL  0x4


typedef struct _NameValue{
	UCHAR name[100];
	ULONG type;
	PWCHAR value;
}NameValue,*PNameValue;
typedef  struct _Name
{
	WCHAR name[100];
	ULONG type;
	ULONG length;

}Name,*PName;

#define CAREFILETABLE_LENGTH 30
#define DEBUG  0x1
#define SF_LOG_PRINT( _dbgLevel, _string )  (FlagOn(SfDebug,(_dbgLevel)) ? DbgPrint _string : ((void)0))
#define  CR_DEBUG(flag,_string) ((flag==0x1)? DbgPrint _string:((void)0))


ULONG    CR_HashFunction(ULONG index);
VOID     CR_MyNodeLock(PHASH_TABLE_CAREFILE_NODE node);
VOID     CR_MyNodeUnLock(PHASH_TABLE_CAREFILE_NODE node);
VOID CR_CareFileTable_Insert(PCareFile carefile);
//这个是初始化关心文件的哈希表，由于比较特殊，所以单独作为一个函数
VOID CR_CareFileTable_CleanUp();
VOID InitHashTable_CareFile();
NTSTATUS CR_SetCareFileInit(PIRP irp);
NTSTATUS  CR_StartInitCareFileHashTable(PIRP irp);
NTSTATUS CR_InitFinished(PIRP irp);
BOOLEAN  CR_CheckIfCare(PCareFile a);