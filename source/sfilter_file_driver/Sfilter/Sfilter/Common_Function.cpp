
#include "stdafx.h"


extern NPAGED_LOOKASIDE_LIST looksidelist; 
extern NPAGED_LOOKASIDE_LIST looksidelist_result;
extern HASH_TABLE hashtable; 
extern HASH_TABLE hashtable_result;
extern PThreadEvents myevents[HASH_TABLE_LENGTH];
extern KSPIN_LOCK myevents_lock;
extern KIRQL myevents_irql;
extern POperationResult resultfortest;
extern size_t process_name_offset;
extern HASH_TABLE_CAREFILE  hashtable_carefile;
VOID KeMyEventsLock(ULONG number);
VOID KeMyEventsUnLock(ULONG number);
VOID HashTableNodeListLock(PHASH_TABLE_NODE node);
VOID HashTableNodeListUnLock(PHASH_TABLE_NODE node);
unsigned short HashFunction_ForOperation(ULONG irptype);
BOOLEAN IsCreateNewFile(PIRP irp);
ULONG  PsGetCurrentProcessName(OUT PUNICODE_STRING name,int *flag);
VOID PlayCurrentThreadName();
VOID DisPlayHashTable(ULONG whichtable);

VOID HashTableNodeListLock(PHASH_TABLE_NODE node)
{
	if(node->initilzed==false)
	{
		if(node->OperationType==-1)
		{
		ASSERT(node->initilzed);
		KdPrint(("hashtable node is not init!\n"));
		return;
		}
	}
	KeAcquireSpinLock(&(node->node_lock),&(node->node_irql));
}
VOID HashTableNodeListUnLock(PHASH_TABLE_NODE node)
{
	if(node->initilzed==false)
	{
		if(node->OperationType==-1)
		{
		ASSERT(node->initilzed);
		KdPrint(("hashtable node is not init!\n"));
		return;
		}
	}
	KeReleaseSpinLock(&node->node_lock,node->node_irql);
}
/*unsigned short HashFunction_ForFile(PFILE_OBJECT fileobject)
{
   ULONG number=(ULONG)fileobject;
    number=number>0?number:(-number);
   return (number%30);
}*/
unsigned short HashFunction_ForOperation(ULONG irptype)  //简单的返回
{
   return irptype%HASH_TABLE_LENGTH;
}
/*VOID MyAddToHashTable(PFILE_OBJECT fileobject,PKSPIN_LOCK spn,PUNICODE_STRING filename)
{
	unsigned short number=HashFunction_ForFile(fileobject);
	if(number>30 ||number <0 ||number ==30)
	{
		KdPrint(("HashFunction error:%d",number));
		return ;
	}
	PHASH_TABLE_NODE hashnode=&(hashtable.link[number]);
	PLIST_ENTRY listentryofhashtable=&(hashnode->entry);

	PAll_DIRECTORS directory=(PAll_DIRECTORS)::ExAllocatePool(NonPagedPool,sizeof(All_DIRECTORS));
	if(directory!=NULL)
	{
		KdPrint(("filename:%wZ\n",filename));
		directory->FileObj=fileobject;
		::RtlCopyUnicodeString(&directory->filename,filename);
		
		::ExInterlockedInsertHeadList(listentryofhashtable,&directory->link,spn); 
		hashnode->number++;
	}
	else
		KdPrint(("directory failed allocate in hashtable!\n"));
    
}*/
/*VOID MyAddToLinkSet(PFILE_OBJECT fileobject,PKSPIN_LOCK spn)
{
	PAll_DIRECTORS directory=(PAll_DIRECTORS)::ExAllocatePool(NonPagedPool,sizeof(All_DIRECTORS));
	if(directory!=NULL)
	{
		KdPrint(("filename:%wZ\n",fileobject->FileName));
		directory->FileObj=fileobject;
		
		
		::ExInterlockedInsertHeadList(&listentry,&directory->link,spn);
	}
	else
		KdPrint(("directory failed allocate!\n"));
}*/

/*VOID HASHTABLETEST()
{
	for(int i=0;i<HASH_TABLE_LENGTH;i++)
	{
		KdPrint(("hashnode[%d]->number==%d",i,hashtable.link[i].number));
	}
}*/

BOOLEAN IsCreateNewFile(PIRP irp)
{
	PIO_STACK_LOCATION ispsp=::IoGetCurrentIrpStackLocation(irp);
	ULONG createdispostion=(ispsp->Parameters.Create.Options>>24)&0x000000ff;

	if(createdispostion==FILE_CREATE || createdispostion==FILE_OPEN_IF ||createdispostion==FILE_OVERWRITE_IF)
	{
		return true;
	}
	else
		return false;
}
VOID KeMyEventsLock(ULONG number)
{
	if(number>=HASH_TABLE_LENGTH || number <0)
		return;
	KeAcquireSpinLock(&myevents[number]->nodelock,&myevents[number]->oldirql);
}

VOID KeMyEventsUnLock(ULONG number)
{
	if(number>=HASH_TABLE_LENGTH || number <0)
		return;
	
	KeReleaseSpinLock(&myevents[number]->nodelock,myevents[number]->oldirql);
}
ULONG  PsGetCurrentProcessName(OUT PUNICODE_STRING name,int *flag)
{
	ULONG need_len;
	ANSI_STRING currname;
	PEPROCESS process;
	process=::PsGetCurrentProcess();
	
	if(process_name_offset==0)
	{
		KdPrint(("process_name_offset is failed!\n"));
		return 0;
	}
	::RtlInitAnsiString(&currname,((PCHAR)process+process_name_offset));
	need_len=RtlAnsiStringToUnicodeSize(&currname);
	if(need_len>name->MaximumLength)
	{
		*flag=0;
		return need_len;
	}
	
	need_len=::RtlAnsiStringToUnicodeString(name,&currname,FALSE);
	*flag=1;
	return need_len;
}
VOID PlayCurrentThreadName()
{
	WCHAR name_buffer[32];			  	  
			UNICODE_STRING processname;
			int flag=0;
			::RtlInitEmptyUnicodeString(&processname,name_buffer,sizeof(WCHAR)*32);
			PsGetCurrentProcessName(&processname,&flag);
			 if(flag==0)
			 {
				 KdPrint(("get process name failed!\n\n"));
			 }
			 else
				 KdPrint(("current process name:%wZ\n",&processname));
}
VOID DisPlayHashTable(ULONG whichtable)
{

	KdPrint(("--------------开始打印哈希表的信息 current irql:%d----------------\n\n",::KeGetCurrentIrql()));
	PHASH_TABLE_NODE pnode;
	PLIST_ENTRY flink;

	PLIST_ENTRY phead;
	int m=1;
	switch(whichtable)
	{
	case(1):
		KdPrint(("OperationInformation HashTable:\n\n"));
		for(int i=0;i<HASH_TABLE_LENGTH;i++)
		{
			pnode=&(hashtable.link[i]);
			::HashTableNodeListLock(pnode);
			KdPrint(("NODE %d: operationtype :%d\n",i,pnode->OperationType));
			phead=&pnode->entry;
			for(flink=phead->Flink;flink!=phead;flink=flink->Flink)
			{
				if(::KeGetCurrentIrql==PASSIVE_LEVEL)
				{
				POperationInformation info=(POperationInformation)CONTAINING_RECORD(flink,OperationInformation,listentry);
				KdPrint(("-------------[%d] node: operationID:%d  filename:%wZ wasdealwith:%d\n",m++,info->OperationID,info->FileName,info->wasdealwith?1:0));
				}
			}

			::HashTableNodeListUnLock(pnode);
		}
		KdPrint(("打印完毕！\n"));
		break;
	case(2):
		KdPrint(("OperationResult HashTable:\n\n"));
		for(int i=0;i<HASH_TABLE_LENGTH;i++)
		{
			pnode=&(hashtable_result.link[i]);
			::HashTableNodeListLock(pnode);
			KdPrint(("NODE %d: operationtype :%d\n",i,pnode->OperationType));
			phead=&pnode->entry;
			for(flink=phead->Flink;flink!=phead;flink=flink->Flink)
			{
				POperationResult info=(POperationResult)CONTAINING_RECORD(flink,OperationResult,entry);
				
				KdPrint(("-------------[%d] node: operationID:%d operation_permit:%d\n",m++,info->operationID,info->operation_permit?1:0));
				
			}

			::HashTableNodeListUnLock(pnode);
		}
		KdPrint(("打印完毕！\n"));
	case(3):
		PHASH_TABLE_CAREFILE_NODE pnode;
		KdPrint(("CareFile HashTable:\n"));
		for(int i=0;i<hashtable_carefile.Length;i++)
		{
			
			pnode=&(hashtable_carefile.link[i]);
			::CR_MyNodeLock(pnode);
			KdPrint(("NODE %d: firstchar :%d \n",i,pnode->firstchar));
			phead=&pnode->entry;
			for(flink=phead->Flink;flink!=phead;flink=flink->Flink)
			{
				PCareFile info=(PCareFile)CONTAINING_RECORD(flink,CareFile,entry);
				
				KdPrint(("-------------[%d] node: filename:(%ws)%wZ filepath:%wZ \n",m++,info->filename.Buffer,&info->filename,&info->filepath));
				
			}

			::CR_MyNodeUnLock(pnode);
		}
		KdPrint(("打印完毕！\n"));
		break;
	default:
		KdPrint(("无法识别的参数！\n"));
		break;
	}
}

//****************这些关于字符的操作用于读取配置文件*********

BOOLEAN CompareUCharString(PUCHAR start, PUCHAR  string,ULONG length)//从指定的位置开始是否有string存在
{
	if(length<0)
		return false;
	
	PUCHAR temp=start;
	while(length!=0)
	{
		if(*temp!=*string)
		{
			KdPrint(("*temp:%c != *string:%c",*temp,*string));
			return false;
		}
		temp++;
		string++;
		length--;
	}
	return true;
}
BOOLEAN CompareWCharString(PWCHAR start,PWCHAR string,ULONG length)
{
		if(length<0)
		return false;
	
	PWCHAR temp=start;
	while(length!=0)
	{
		if(*temp!=*string)
		{
		//	KdPrint(("*temp:%ws != *string:%ws",*temp,*string));
			return false;
		}
		temp++;
		string++;
		length--;
	}
	return true;
}
ULONG MyFileFullPathQuery(IN PFILE_OBJECT file,OUT PUNICODE_STRING path)
{
	NTSTATUS status;
	POBJECT_NAME_INFORMATION nameinfo=NULL;
	
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
			return STATUS_INSUFFICIENT_RESOURCES;
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
	if(path->MaximumLength<length)
	{
		KdPrint(("length is not enough :%d",path->MaximumLength));
		
		break;
	}
	::RtlCopyUnicodeString(path,&nameinfo->Name);
	//KdPrint(("devicenameinfo:%wZ path:%wZ",nameinfo->Name,path));
	if(need_split)
		//::RtlAppendUnicodeToString(path,L"\\\n");
		;
		RtlAppendUnicodeStringToString(path,&file->FileName);
	
	//KdPrint(("filename:%wZ path:%wZ",&file->FileName,path));
	}while(0);
	if((void*)nameinfo!=(void*)buf)
		  ExFreePool(nameinfo);
	//KdPrint(("MyFileQuery Out :%wZ",path));
	return length;

}
VOID  GetFileNameFromDirectory(PUNICODE_STRING directory,OUT PUNICODE_STRING filename)
{
	if(directory->Length==0)
		return ;
	PWCHAR temp=directory->Buffer;
	PWCHAR start=directory->Buffer;
	PWCHAR end=directory->Buffer+(directory->Length)/2;
	WCHAR filename_buffer[FILE_NAME_LENGTH_MAX]={0};

	KdPrint(("GetFileName from %wZ \n",directory));
	for(int i=0;i<directory->Length/2;i++)
	{
		WCHAR seg=L'\\';
	   
		if(start[i]==seg)
		{
			temp=start+i;
		}
		if(i==(directory->Length/2)-1) //到达尾部
		{						
			int m=0;
			temp++;
			PWCHAR sta=filename->Buffer;
			while(temp!=end)
			{
				*sta=*temp;
				sta++;
				temp++;
				m++;
			}
			*sta=*end;
			filename->Length=(m+1)*2;
		}
		
	}
	KdPrint(("GetFileName Finished:%wZ!\n",filename));
}
BOOLEAN EumnSubKey(PUNICODE_STRING  srt)
{
	
	HANDLE registry;
	OBJECT_ATTRIBUTES objectatt;
	InitializeObjectAttributes(&objectatt,srt,OBJ_CASE_INSENSITIVE,NULL,NULL);

	NTSTATUS status=STATUS_SUCCESS;
	status=::ZwOpenKey(&registry,KEY_ALL_ACCESS,&objectatt);
	if(!NT_SUCCESS(status))
	{
		KdPrint(("打开%wZ失败\n",srt));
		return false;
	}
	ULONG ulsize;

	status=::ZwQueryKey(registry,KeyFullInformation,NULL,0,&ulsize);
	PKEY_FULL_INFORMATION info=(PKEY_FULL_INFORMATION)::ExAllocatePool(NonPagedPool,ulsize);
	status=::ZwQueryKey(registry,KeyFullInformation,info,ulsize,&ulsize);
	if(NT_SUCCESS(status))
	{
		KdPrint(("__________开始枚举子项(%d)________\n",info->SubKeys));
		for(int i=0;i<info->SubKeys;i++)
		{
			::ZwEnumerateKey(registry,i,KeyBasicInformation,NULL,0,&ulsize);

			PKEY_BASIC_INFORMATION pb=(PKEY_BASIC_INFORMATION)::ExAllocatePool(NonPagedPool,ulsize);

			::ZwEnumerateKey(registry,i,KeyBasicInformation,pb,ulsize,&ulsize);
			UNICODE_STRING unikeyname;
			unikeyname.Length=unikeyname.MaximumLength=(USHORT)pb->NameLength;
			unikeyname.Buffer=pb->Name;			
			KdPrint(("the %d sub item name:%ws\n",i,&unikeyname.Buffer));

			::ExFreePool(pb);
		}
	}
	else
		status=NULL;
	ExFreePool(info);
	ZwClose(registry);
	return status;
}
NTSTATUS QueryRoutine (IN PWSTR ValueName, IN ULONG ValueType,IN PVOID ValueData, IN ULONG ValueLength,IN PVOID Context,IN PVOID EntryContext )
{
	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(ValueLength);
	UNREFERENCED_PARAMETER(ValueType);
	UNREFERENCED_PARAMETER(EntryContext);
	UNREFERENCED_PARAMETER(ValueData);
	UNREFERENCED_PARAMETER(ValueName);
	KdPrint(("开始打印查询键的信息:\r\n"));
	KdPrint(("ValueName:%ws--\n ValueData:%ws\n",ValueName,(PWSTR)ValueData));
	KdPrint(("查询结果为:%ws\n",(PWSTR)EntryContext));
	return STATUS_SUCCESS;
}
BOOLEAN   Query_SymbolLinkInfo()
{
	KdPrint(("开始查询 符号链接信息!\n"));
	 NTSTATUS  status;
	// RTL_QUERY_REGISTRY_TABLE querytable[2];

    WCHAR* pbuf=(WCHAR*)ExAllocatePool(NonPagedPool,100);
	RtlZeroMemory(pbuf,100);
	//RtlZeroMemory(&querytable,sizeof(querytable)); 
	
	//querytable[0].QueryRoutine=(PRTL_QUERY_REGISTRY_ROUTINE)QueryRoutine;
	//querytable[0].Flags=RTL_QUERY_REGISTRY_NOEXPAND;
	//querytable[0].Name=NULL;
	//querytable[0].EntryContext=pbuf;
	
	//status=::RtlQueryRegistryValues(RTL_REGISTRY_CONTROL,L"\\Session Manager\\DOS Devices",querytable,NULL,NULL);

	UNICODE_STRING tosub;
	::RtlInitUnicodeString(&tosub,L"\\Registry\\Machine\\System\\MountedDevices");
	EumnSubKey(&tosub);

	//+++++++++++++++++++++++++++++++++++++++
	HANDLE registry;
	OBJECT_ATTRIBUTES objectatt;
	InitializeObjectAttributes(&objectatt,&tosub,OBJ_CASE_INSENSITIVE,NULL,NULL);	
	status=::ZwOpenKey(&registry,KEY_ALL_ACCESS,&objectatt);
	if(!NT_SUCCESS(status))
	{
		KdPrint(("打开%wZ失败\n",&tosub));
		return false;
	}
	else
		KdPrint((""));
	if(!NT_SUCCESS(status))
	{
		KdPrint(("查询键信息失败!\n"));
		return false;
	}
	else
		return true;
}