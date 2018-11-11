#include "stdafx.h"

#define FILE_PATH_LENGTH   150   //绝大部分的路径长度不会超过这个值,操作系统本身没有对路径作长度要求

#define START_CHAR   L'*'
#define END_CHAR     L'?'
#define SEG_CHAR     L'|'
#define DEF_CHAR	 L':'

extern NPAGED_LOOKASIDE_LIST looksidelist; 
extern NPAGED_LOOKASIDE_LIST looksidelist_result;
extern HASH_TABLE hashtable; 
extern HASH_TABLE hashtable_result;
extern PThreadEvents myevents[HASH_TABLE_LENGTH];


extern POperationResult resultfortest;
extern size_t process_name_offset;

extern NPAGED_LOOKASIDE_LIST  looksidelist_carefile;
extern HASH_TABLE_CAREFILE hashtable_carefile;

extern BOOLEAN CareFile_Initilized;

KSPIN_LOCK lock;
KIRQL lock_irql;

Name myname[Name_Value_Number];
//***************************由于在内核下面的路径与应用成有些不同，所以，需要在注册表中取得一些信息并存储起来*****************


ULONG    CR_HashFunction(ULONG index)//根据给定的键值计算下标
{
	if(index<0)       //这是针对英文的开头的，文件名可能是中文开头，这时候就取模
	{
		return MATCHING_ERROR_TYPE_ONE ;
	}
	else
		if(index>26)
			return MATCHING_ERROR_TYPE_TWO;
		else
		{
			return index;
		}

}
VOID     CR_MyNodeLock(PHASH_TABLE_CAREFILE_NODE node)
{
	if(node==NULL)
		return ;
	KeAcquireSpinLock(&node->node_lock,&node->node_irql);
}
VOID     CR_MyNodeUnLock(PHASH_TABLE_CAREFILE_NODE node)
{
	if(node==NULL)
		return ;
	KeReleaseSpinLock(&node->node_lock,node->node_irql);
}
VOID CR_CareFileTable_Insert(PCareFile carefile)
{
  CR_DEBUG(DEBUG,("CareFileTable_Insert!\n\n"));
  UCHAR chars=carefile->filepath.Buffer[0];
  WCHAR test='c';
  
  ULONG index=carefile->filepath.Buffer[0]>=L'a'?(carefile->filepath.Buffer[0]-L'a'):carefile->filepath.Buffer[0]-L'A';
  CR_DEBUG(DEBUG,("CareFileTable_Insert:index(%d=%d(%c&%wc)-%d) -%d -%d-%c!\n",index,carefile->filepath.Buffer[0],carefile->filepath.Buffer[0],carefile->filepath.Buffer[0],L'A','A',test,test));
 
  ULONG index_number=::CR_HashFunction(index);
  
  PHASH_TABLE_CAREFILE_NODE pnode =&hashtable_carefile.link[index_number];//如果匹配不了则填入最后一个节点
  CR_MyNodeLock(pnode);
	 
  pnode->totalnumbers++;
  pnode->number++;

  ::InsertHeadList(&pnode->entry,&carefile->entry);

  ::CR_MyNodeUnLock(pnode);

  CR_DEBUG(DEBUG,("leave CareFileTable_Insert !\n\n"));



}
//这个是初始化关心文件的哈希表，由于比较特殊，所以单独作为一个函数
VOID CR_CareFileTable_CleanUp()
{
	PHASH_TABLE_CAREFILE_NODE filenode;
	for(int j=0;j<HASH_TABLE_LENGTH;j++)
	{
		
		filenode=&(hashtable_carefile.link[j]);
		PLIST_ENTRY entry=&(filenode->entry);
		::CR_MyNodeLock(filenode);
		PCareFile carefile;
		while(!IsListEmpty(entry))
		{
			
			PLIST_ENTRY temp=::RemoveTailList(entry);
			carefile=CONTAINING_RECORD(temp,CareFile,entry);
			::ExFreePool(carefile->filename.Buffer);
			::ExFreePool(carefile->filepath.Buffer);
			::ExFreeToNPagedLookasideList(&looksidelist_carefile,carefile);
		}
		::CR_MyNodeUnLock(filenode);
	}
}
VOID InitHashTable_CareFile()
{
	//刚开始初始化，驱动还没有开始工作，所以使用zwcreatefile来获得文件句柄，否则需要自己创建irp来向底层设备发起读文件的请求
	//所以这个函数必须放在DriverEntry里面.
	CR_DEBUG(DEBUG,("开始初始化关心文件哈希表相关变量  r4！\n\n"));
	HANDLE file;
	::KeInitializeSpinLock(&lock);

	OBJECT_ATTRIBUTES objectattr;
	UNICODE_STRING filename;
	IO_STATUS_BLOCK statusblock;
	NTSTATUS status;
	ULONG length;
	ULONG filenumber=0;
	FILE_STANDARD_INFORMATION fsi;

	hashtable_carefile.Length=CAREFILETABLE_LENGTH;
	for(int i=0;i<hashtable_carefile.Length;i++)
	{
		PHASH_TABLE_CAREFILE_NODE pnode=&(hashtable_carefile.link[i]);
		::InitializeListHead(&pnode->entry);
		::KeInitializeSpinLock(&pnode->node_lock);
		pnode->firstchar=L'a'+i;
		pnode->NodeID=i;
		pnode->number=0;
		pnode->totalnumbers=0;
		pnode->initilzed=true;
	}
	CR_DEBUG(DEBUG,("HASHTABLE_CAREFILE结构初始化完毕,开始读取数据!\n\n"));

	for(int j=0;j<Name_Value_Number;j++)
	{
		RtlZeroMemory(&myname[j],sizeof(Name));
	    switch(j)
		{
		case(0):
			memcpy(&myname[j].name,L"filename",100);
			myname[j].type=NAME_VALUE_TYPE_FILENAME;
			myname[j].length=8;
			break;
		case(1):
			memcpy(&myname[j].name,L"filepath",100);
			myname[j].type=NAME_VALUE_TYPE_FILEPATH;
			myname[j].length=8;
			break;
		case(2):
			memcpy(&myname[j].name,L"security_level",100);
			myname[j].type=NAME_VALUE_TYPE_SECURITY_LEVEL;
			myname[j].length=14;
			break;
		default:
			break;
		}
	}
	
	CareFile_Initilized=false;
	::RtlInitUnicodeString(&filename,L"\\??\\C:\\Windows\\carefile.dat");
	InitializeObjectAttributes(&objectattr,
		                        &filename,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL);

	status=::ZwOpenFile(&file,
		FILE_ANY_ACCESS,
		&objectattr,
		&statusblock,
		0,
		FILE_NON_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT);

	if(NT_SUCCESS(status))
	{
		CR_DEBUG(DEBUG,("成功打开文件!\n"));

	}
	else
	{
		CR_DEBUG(DEBUG,("打开文件失败!\n"));
		::ZwClose(file);
	
		return ;
	}

	status=::ZwQueryInformationFile(file,
		&statusblock,
		&fsi,
		sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation);
	if(!NT_SUCCESS(status))
	{
		CR_DEBUG(DEBUG,("查询信息失败\n"));
		::ZwClose(file);

		return;
	}
	PWCHAR pbuffer=(PWCHAR)::ExAllocatePool(NonPagedPool,(ULONG)fsi.EndOfFile.QuadPart);

	status=::ZwReadFile(file,
		NULL,
		NULL,
		NULL,
		&statusblock,
		pbuffer,
		(LONG)fsi.EndOfFile.QuadPart,
		NULL,
		NULL);

	if(!NT_SUCCESS(status))
	{
		CR_DEBUG(DEBUG,("读取文件信息失败!\n"));
		::ZwClose(file);
	   ::ExFreePool(pbuffer);
		return;
	}
	CR_DEBUG(DEBUG,("文件内容：type:%c \n%ws",*pbuffer,pbuffer));
	//***读取完毕，开始提取信息*********
	length=(ULONG)fsi.EndOfFile.QuadPart;
	PCareFile filenode;

	
	UCHAR name[100]={0};
	
	PWCHAR temp=pbuffer;
	Query_SymbolLinkInfo();
	//WCHAR filenamebuffer[1024];
	CR_DEBUG(DEBUG,("开始提取信息!\n\n"));
	ULONG len=0; //记录pbuffer指针的当前位置
	PWCHAR start=pbuffer;  //保存这个头指针，用于释放该内存

	NameValue namevalue[Name_Value_Number];   //这个是分配给path和filename的buffer的
	for(int i=0;i<=length;i=len)
	{

		if(*pbuffer==START_CHAR)
		{
			//filenumber++;
			//node=(PCareFile)::ExAllocateFromNPagedLookasideList(&looksidelist_carefile);
			temp=pbuffer;
			len++;
			if(*(++pbuffer)=START_CHAR)       //找到了起始符号
			{
			   CR_DEBUG(DEBUG,("找到一个开始符:%d\n",filenumber));
			   filenumber++;
			   	ULONG namenumber=0;  //记录的是第几个name
				

			   while(!(*pbuffer==END_CHAR &&*(--pbuffer)==END_CHAR) && len<=length &&namenumber< Name_Value_Number)             //直到找到结束符号为止,如果超过了length或者超过了最大的值对数量也是返回
			   {
				   CR_DEBUG(DEBUG,("找到一个符:%ws\n",*pbuffer));
				  if(CompareWCharString(++pbuffer,myname[namenumber].name,myname[namenumber].length))//找到name
			      {
				   CR_DEBUG(DEBUG,("找到一个Name:%ws \n",myname[namenumber].name));
				   temp=pbuffer;
				   pbuffer+=myname[namenumber].length;//pbuffer移动到:之后，也就是值得第一个字符
				   len+=myname[namenumber].length;
				   if(*pbuffer!=DEF_CHAR)         //没有找到值对的分隔符:，则跳出循环
				   {
					   	CR_DEBUG(DEBUG,("没有找到值对的分隔符:(*pbuffer==%c)\n",*pbuffer));
					   goto error;
				   }
				   int m=0;
				   BOOLEAN flag=TRUE; //这个标志没有超出过FILE_PATH_LENGTH长度
				   WCHAR value[MAX_FILE_PATH_LENGTH]={0};
				   pbuffer++;
				   len++;
				   //动态分配保存了文件路径，如果超过了预定的长度，那么就重新分配
				   KdPrint(("分配了nonpagedpool 内存!\n"));
				   namevalue[namenumber].value=(PWCHAR)::ExAllocatePool(NonPagedPool,sizeof(WCHAR)*FILE_PATH_LENGTH);

				   while((*pbuffer!=SEG_CHAR&& *pbuffer!=END_CHAR)&& len<=length &&namenumber< Name_Value_Number )   //记录值得内容直到遇到当前值对的结束符 %
				   {
					   value[m]=*pbuffer;
					   m++;
					   pbuffer++;
					   len++;
					   if(m>=FILE_PATH_LENGTH-1 && flag)//缓冲区已经用完
					   {
						   PWCHAR tempx=namevalue[namenumber].value;
						   namevalue[namenumber].value=(PWCHAR)::ExAllocatePool(NonPagedPool,sizeof(WCHAR)*FILE_PATH_LENGTH_MAX);						
						   ::ExFreePool(tempx);
						   flag=FALSE;

					   }
					   if(m>=FILE_PATH_LENGTH_MAX-1)
						   break;
				    }
				   value[m]=0;
				   namevalue[namenumber].type=myname[namenumber].type;
				   wcsncpy(namevalue[namenumber].value,value,flag?FILE_PATH_LENGTH:MAX_FILE_PATH_LENGTH);
				   
				   CR_DEBUG(DEBUG,("找到当前对应的Value:%ws\n %ws\n",value,namevalue[namenumber].value));
				   namenumber++;
			    }

			   else
			     {
				   CR_DEBUG(DEBUG,("匹配Name出错!\n\n"));
				   goto error;
				  }
			 }
			   //****************************到这里找到了一个文件记录*************
			   CR_DEBUG(DEBUG,("开始写入文件记录:%d\n",filenumber));
			   filenode=(PCareFile)::ExAllocateFromNPagedLookasideList(&looksidelist_carefile);
			   
			   for(int k=0;k<Name_Value_Number;k++)
			   {
				   CR_DEBUG(DEBUG,("namevalue[%d]:%ws  %d\n",k,namevalue[k].value,namevalue[k].type));
				   switch(namevalue[k].type)
				   {
				   case(NAME_VALUE_TYPE_FILENAME):				
					   RtlInitUnicodeString(&filenode->filename,namevalue[k].value);//这里进行的强制转换，可能会出现问题
					   KdPrint(("filename:%ws --%d--\n--%ws--\n",filenode->filename.Buffer,k,namevalue[k].value));
					   break;
				   case(NAME_VALUE_TYPE_FILEPATH):
					   ::RtlInitUnicodeString(&filenode->filepath,(PCWSTR)namevalue[k].value);
					   CR_DEBUG(DEBUG,("filepath:%ws %d\n %ws\n",filenode->filepath.Buffer,k,namevalue[k].value));					  
					   break;

				   case(NAME_VALUE_TYPE_SECURITY_LEVEL):
					   filenode->secutiry_level=namevalue[k].value[0]-'0';  
					   CR_DEBUG(DEBUG,("level:%d\n",k,filenode->secutiry_level));
					   break;
				   default:
					   CR_DEBUG(DEBUG,("DEFAUKLT\n"));
					   break;
				   }
			   }
			   ::CR_CareFileTable_Insert(filenode);

			}
			else
			{
				CR_DEBUG(DEBUG,("没有找到第二个起始符:# \n\n"));
				goto error;
			}
		}
		len++;
		
		temp=pbuffer;      
		pbuffer++;
		i=len;
		
	}
	/*CR_DEBUG(DEBUG,("THIS IS A TEST :\n"));
	PWCHAR a=(PWCHAR)::ExAllocatePool(NonPagedPool,sizeof(WCHAR)*FILE_PATH_LENGTH);
	wcsncpy(a,L"sdawdacsa",wcslen(L"sdawdacsa"));
	KdPrint(("sdad:%ws\n",a));
	UNICODE_STRING  my;
	UNICODE_STRING mys;
	my.Buffer=a;
	my.Length=wcslen(L"sdawdacsa");
	my.MaximumLength=FILE_PATH_LENGTH;
	::RtlInitUnicodeString(&mys,a);
	KdPrint(("sdadsa:%ws\n",a));
	KdPrint(("string :%ws   %d  %d\n\n",my.Buffer,my.Length,my.MaximumLength));
	KdPrint(("string :%ws   %d  %d\n\n",mys.Buffer,mys.Length,mys.MaximumLength));
	
	KIRQL old=::KeGetCurrentIrql();
	KdPrint(("current level :%d",old));
	KeRaiseIrql(DISPATCH_LEVEL,&old);
	KdPrint(("string :%ws   %d  %d\n\n",mys.Buffer,mys.Length,mys.MaximumLength));
	KdPrint(("string :%wZ ",mys));
	KeLowerIrql(old);
	*/
	::DisPlayHashTable(3);
	CareFile_Initilized=true;
	::ZwClose(file);
	::ExFreePool(start);
	return ;
	error:
	CareFile_Initilized=false;
	CR_DEBUG(DEBUG,("初始化CareFile时遇到错误 %s\n\n"));	
	ZwClose(file);
	ExFreePool(start);
	::CR_CareFileTable_CleanUp();                //出现错误就清除已经分配的所有内存
}
NTSTATUS CR_SetCareFileInit(PIRP irp)
{
	KeAcquireSpinLock(&lock,&lock_irql);
	
	PIO_STACK_LOCATION irpsp=::IoGetCurrentIrpStackLocation(irp);
	PKEVENT waitevent;
	HANDLE waiteventhandle;
	waiteventhandle=*(HANDLE*)irp->AssociatedIrp.SystemBuffer;
	NTSTATUS status;

	status=ObReferenceObjectByHandle(waiteventhandle,
		EVENT_MODIFY_STATE,
		*ExEventObjectType
		,KernelMode,
		(PVOID*)&waitevent,
		NULL);
	if(!NT_SUCCESS(status))
	{

		CR_DEBUG(DEBUG,("获取内核对象失败(CR_SetCareFileInit)!\n\n"));
		return NULL;
	}
	BOOLEAN init;
	init=(BOOLEAN)MmGetSystemAddressForMdlSafe(irp->MdlAddress,NormalPagePriority);

	init=CareFile_Initilized;
	CR_DEBUG(DEBUG,("设置:内和初始化状态:INIT=%d Return= %d",CareFile_Initilized?1:0,init?1:0));
	::KeSetEvent(waitevent,IO_NO_INCREMENT,FALSE);

	KeReleaseSpinLock(&lock,lock_irql);
	
	CR_DEBUG(DEBUG,("leave from CR_SetCareFileInit irq：%d !\n",::KeGetCurrentIrql()));
	return status;
}
NTSTATUS  CR_StartInitCareFileHashTable(PIRP irp)//开始初始化关心文件链表
{
	
	CR_DEBUG(DEBUG,("CR_StartInitCareFileHashTable! irql:%d\n",KeGetCurrentIrql()));

	PCareFile node;          //节点
	HANDLE waitevent;
	PCareFileTransfer in=0;
	NTSTATUS status=STATUS_ACCESS_DENIED;
	KIRQL      oldirql;                   //这个函数将会被调用于ring0的环境，所以如果有函数把IRQL提高了，那么必须把IRQL降回来
	oldirql=::KeGetCurrentIrql();
	KIRQL now=oldirql;

	//KeRaiseIrql(DISPATCH_LEVEL,&oldirql);
	waitevent=*(HANDLE*)irp->AssociatedIrp.SystemBuffer;
	if(waitevent==NULL)
		return status;
	status=ObReferenceObjectByHandle(waitevent,
		EVENT_MODIFY_STATE,
		*ExEventObjectType
		,KernelMode,
		(PVOID*)&waitevent,
		NULL);
	if(!NT_SUCCESS(status))
		return status;
	
	in=(PCareFileTransfer)MmGetSystemAddressForMdlSafe(irp->MdlAddress,NormalPagePriority);

	if(in==NULL)
		return status;
	CR_DEBUG(DEBUG,("开始接收数据!:%d\n",in->number));
	PCareFile_T innode=0;
	for(int m=0;m<in->number;m++)
	{
		CR_DEBUG(DEBUG,("-----The %d current irql:%d\n",m,::KeGetCurrentIrql()));

		node=(PCareFile)::ExAllocateFromNPagedLookasideList(&looksidelist_carefile);
		if(node==NULL)
			break;
		if(in->filenode!=NULL)
		{
			innode=&in->filenode[m];
			CR_DEBUG(DEBUG,("filenode %d: %ws",m,innode->filename));
			::RtlInitUnicodeString(&node->filename,innode->filename);
			::RtlInitUnicodeString(&node->filepath,innode->filepath);
			node->Owner=innode->Owner;
			node->secutiry_level=innode->secutiry_level;			
		}
		else
			break;
	}

//	KeLowerIrql(oldirql);
	CR_DEBUG(DEBUG,("Leave from CR_StartInitCareFileHashTable!\n\n"));
	::CareFile_Initilized=true;
	return status;;
}
NTSTATUS CR_InitFinished(PIRP irp)
{
	KIRQL level;
	CR_DEBUG(DEBUG,("current irql:%d",::KeGetCurrentIrql()));
	KeRaiseIrql(DISPATCH_LEVEL,&level);

	KeAcquireSpinLock(&lock,&lock_irql);
	PIO_STACK_LOCATION irpsp=::IoGetCurrentIrpStackLocation(irp);
	PKEVENT waitevent;
	HANDLE waiteventhandle;
	waiteventhandle=*(HANDLE*)irp->AssociatedIrp.SystemBuffer;
	NTSTATUS status;

	status=ObReferenceObjectByHandle(waiteventhandle,
		EVENT_MODIFY_STATE,
		*ExEventObjectType
		,KernelMode,
		(PVOID*)&waitevent,
		NULL);
	if(!NT_SUCCESS(status))
	{

		CR_DEBUG(DEBUG,("获取内核对象失败(CR_InitFinished)!\n\n"));
		return NULL;
	}

	KeSetEvent(waitevent,IO_NO_INCREMENT,FALSE);
	
	CR_DEBUG(DEBUG,("Leave from CR_InitFinished!:%d\n",::KeGetCurrentIrql()));
	
	KeLowerIrql(level);
	CR_DEBUG(DEBUG,("current irql:%d level:%d",::KeGetCurrentIrql(),level));
	return status;
}
BOOLEAN  CR_CheckIfCare(PCareFile a)
{
	KdPrint(("CR_CheckIfCare In:%ws!\n",a->filepath.Buffer));
	BOOLEAN flag=FALSE;
	if(a->filepath.Length==0)
		return FALSE;
	
   ULONG index=a->filepath.Buffer[0]>L'a'?a->filepath.Buffer[0]-L'a':a->filepath.Buffer[0]-L'A';
	
   ULONG index_number;

   index_number=::CR_HashFunction(index);

    PHASH_TABLE_CAREFILE_NODE pnode =&hashtable_carefile.link[index_number];//如果匹配不了则填入最后一个节点
	//CR_MyNodeLock(pnode);
	 
	PLIST_ENTRY phead,temp;
	phead=&pnode->entry;    
	
	for(temp=phead->Flink;temp!=phead;temp=temp->Flink)
	{
		PCareFile carefile=CONTAINING_RECORD(temp,CareFile,entry);

		
		if(::CompareWCharString(carefile->filepath.Buffer,a->filepath.Buffer,carefile->filepath.MaximumLength>a->filepath.MaximumLength?a->filepath.MaximumLength:carefile->filepath.MaximumLength))
			{
				
				a->Owner=carefile->Owner;
				a->secutiry_level=carefile->secutiry_level;
				//CR_MyNodeUnLock(pnode);
				return TRUE;
			}
			else
				CR_DEBUG(DEBUG,("mismatch ,filepath:%ws != %ws",a->filepath.Buffer,carefile->filepath.Buffer));
		
	}
//	CR_DEBUG(DEBUG,("mismatch ,filename:%wZ",a->filename));

   // CR_MyNodeUnLock(pnode);
   return FALSE;
}



