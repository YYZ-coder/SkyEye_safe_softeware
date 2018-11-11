#include "stdafx.h"


//****************sfilter.cpp*********************

extern NPAGED_LOOKASIDE_LIST looksidelist; 
extern NPAGED_LOOKASIDE_LIST looksidelist_result;
extern HASH_TABLE hashtable; 
extern HASH_TABLE hashtable_result;
extern PThreadEvents myevents[HASH_TABLE_LENGTH];


extern POperationResult resultfortest;
//**************************************************************************************




NTSTATUS FastIO_GetOperationInformation(ULONG iocontrolcode,PVOID inbuffer,PVOID outbuffer,ULONG inlen,ULONG outlen)
{
	NTSTATUS status=STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(outlen);
	ULONG code=iocontrolcode;
	ULONG in_len=inlen;
	ULONG out_len=outlen;
	ULONG operationtype;   //这是本次感兴趣的操作类型
	HANDLE waiteventhandle;
	PKEVENT waitevent;
	POperationRecord record=(POperationRecord)inbuffer;
	POperationRecord out=(POperationRecord) outbuffer;

	ASSERT(code==IOCTL_VERIFY_OPERATION_INFORMATION);
	ASSERT(in_len==sizeof(POperationRecord));
	ASSERT(out_len==sizeof(POperationRecord));
	ASSERT(inbuffer!=NULL);
	ASSERT(outbuffer!=NULL);

	operationtype=record->operationtype;
	waiteventhandle=record->event;
	do
	{

    if(record->event==NULL||record->forback==NULL)
	{
		KdPrint(("event or forback is null \n"));
		record->exception=TRUE;
		record->exceptiontype=EXCEPTION_APPLICATION_ERROR;
		wcsncpy(record->exceptionmessage,
			L"event or forback is null!\r\n",
			sizeof(L"event or forback is null!\r\n\n")
			);
		break ;
	}
	status=ObReferenceObjectByHandle(waiteventhandle,EVENT_MODIFY_STATE,*ExEventObjectType,KernelMode,(PVOID*)&waitevent,NULL);
	if(!NT_SUCCESS(status))
	{
		//KdPrint(("无效的句柄！\n"));
		record->exception=TRUE;
		record->exceptiontype=EXCEPTION_APPLICATION_ERROR;
		wcsncpy(record->exceptionmessage,
			L"ObReferenceObjectByHandle failed!\r\n",
			sizeof(L"ObReferenceObjectByHandle failed!\r\n\n")
			);
		break;
	}

	//开始从hashtable里面获得OperationInformation结构
	POperationInformation pnode=NULL;
	GetOperationInformationFromHashTable(operationtype,&pnode);
	if(pnode==NULL)
	{
		record->exception=true;
		record->exceptiontype=EXCEPTION_NONE_RECORD;
		wcsncpy(record->exceptionmessage,
			L"there are no OperationInformation anymore!\r\n",
			sizeof(L"there are no OperationInformation anymore!\r\n\n")
			);

		break;
	}
	 //成功取到了这个结构，那么这时候获得用户程序提供的输出缓冲区来写这些信息
	
	if(out!=NULL)
	{
	wcsncpy(out->filename,pnode->FileName.Buffer,pnode->FileName.Length);
	wcsncpy(out->filepath,pnode->FilePath.Buffer,pnode->FilePath.Length);
	out->OperationID=pnode->OperationID;
	out->operationtype=pnode->OperationType;

	}
	pnode=NULL;                       //释放指针
	KdPrint(("write data over\r\n\n"));
	}while(0);
	::KeSetEvent(waitevent,IO_NO_INCREMENT,false);       //设置事件，通知用户已经取完了信息
	::ObfDereferenceObject(waitevent);                   //减少事件引用
	return status;
}

NTSTATUS FastIO_SetOperationResult(ULONG iocontrolcode,PVOID inbuffer,PVOID outbuffer,ULONG inlen,ULONG outlen)
{
    
	UNREFERENCED_PARAMETER(outlen);
	UNREFERENCED_PARAMETER(outbuffer);
	UNREFERENCED_PARAMETER(inlen);
	UNREFERENCED_PARAMETER(iocontrolcode);
	NTSTATUS status=STATUS_ACCESS_DENIED;
	ASSERT(iocontrolcode==IOCTL_SEND_VERIFY_RESULT);
	ASSERT(inlen>=sizeof(OperationResult));
	ASSERT(inbuffer!=NULL);
	BOOLEAN flag=FALSE;

	PVOID buffer=inbuffer;
	POperationResult result;
	POperationResult newresult;

	result=(POperationResult)buffer;
	if(result==NULL)
		return status;
	newresult=(POperationResult)::ExAllocateFromNPagedLookasideList(&looksidelist_result);
	newresult->operationID=result->operationID;
	newresult->operationtype=result->operationtype;
	newresult->operation_permit=result->operation_permit;
	wcsncpy(newresult->otherinfo,result->otherinfo,sizeof(WCHAR)*512);

	 MyOperationResultAddToHashTable2(newresult);

	PHASH_TABLE_NODE tablenode;
	ULONG number=HashFunction_ForOperation(result->operationtype);
	tablenode=&(hashtable.link[number]);
	 	PLIST_ENTRY p,head;
	POperationInformation results;
	head=&tablenode->entry;
	::HashTableNodeListLock(tablenode);
	for(p=head->Flink;p!=head;p=p->Flink)        
	{
        results=(POperationInformation)p;
		if(result->operationID==results->OperationID)
		{
			::KeSetEvent(&results->finished,IO_NO_INCREMENT,FALSE);

			PLIST_ENTRY flink,blink;
			flink=p->Blink;
			blink=p->Flink;
			flink->Flink=blink;
			blink->Blink=flink;
			//::ExFreeToNPagedLookasideList(&looksidelist,results);  //把hashtable里面的对应的operationinformation结构从链表里面释放出来
			flag=TRUE;
			break;
		}
	}
	::HashTableNodeListUnLock(tablenode);
	if(flag)
	{
		KdPrint(("FASTIO_SetOperationResult failed!\n"));
		status=STATUS_SUCCESS;
	}
	
	return status;
}
//**************************************************************************************************
VOID GetOperationInformation(PIRP irp)
{
	NTSTATUS status;
	PIO_STACK_LOCATION irpsp=::IoGetCurrentIrpStackLocation(irp);
	ULONG number=-1;
	ULONG operationtype;   //这是本次感兴趣的操作类型
	HANDLE waiteventhandle; 
	PKEVENT waitevent;
	ULONG concode=irpsp->Parameters.DeviceIoControl.IoControlCode;
	PVOID buffer=irp->AssociatedIrp.SystemBuffer;   //
	ULONG inlen=irpsp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG outlen=irpsp->Parameters.DeviceIoControl.OutputBufferLength;        
	POperationRecord out;
	bool exception=false;
	WCHAR exceptionmessage[512]=L"";
	ASSERT(inlen==sizeof(POperationRecord));
	ASSERT(buffer!=NULL);
	do
	{
	if(buffer==NULL)
	{		
		break ;
	}
	KdPrint(("buffer is not null\n"));
	waiteventhandle=*(HANDLE*)buffer;

	if(waiteventhandle==NULL)
	{
		KdPrint(("event is null!\n"));
		exception=true;
		wcsncpy(exceptionmessage,
			L"event or forback is null!\r\n",
			sizeof(L"event or forback is null!\r\n\n")
			);
	}	
	//********************获得输出缓冲区，如果出现意外，那么就填写好意外信息直接返回;
	KdPrint(("get out buffer:mdladdress!\n"));
	out=(POperationRecord)MmGetSystemAddressForMdlSafe(irp->MdlAddress,NormalPagePriority); 
	if(out!=NULL)
	{
		operationtype=out->operationtype;
		number=HashFunction_ForOperation(operationtype);
		KdPrint(("当前线程所关心操作类型是:%d",operationtype));
	}
	else
	{
		KdPrint(("out buffer is null!\n"));
		break;
	}
	//***************开始查询所关心的类型是否有对应的操作信息需要验证，如果有，就取出，否则就记录事件，然后等待，直到有，然后通知应用来取*****************
	status=ObReferenceObjectByHandle(waiteventhandle,EVENT_MODIFY_STATE,*ExEventObjectType,KernelMode,(PVOID*)&waitevent,NULL);
	if(!NT_SUCCESS(status))//如果失败，那么就获得输出缓冲区里的event事件，一般这个不会出错，如果出错，程序崩溃
	{
		KdPrint(("event from InputBuffer have occured a error when it is used to get a kernel object by useing ObReferenceObjectByHandle!\r\n\n"));
		exception=true;
		wcsncpy(exceptionmessage,
			L"event from InputBuffer have occured a error when it is used to get a kernel object by useing ObReferenceObjectByHandle!\r\n",
			sizeof(L"event from InputBuffer have occured a error when it is used to get a kernel object by useing ObReferenceObjectByHandle!\r\n\n")
			);
		KdPrint(("start ObReferenceObjectByHandle in POperationRecord\n\n"));
		waiteventhandle=out->event;
		status=ObReferenceObjectByHandle(waiteventhandle,EVENT_MODIFY_STATE,*ExEventObjectType,KernelMode,(PVOID*)&waitevent,NULL);
	    if(!NT_SUCCESS(status))    //如果获得事件的内核对象失败了，那么直接返回失败
	   {
		 KdPrint(("无效的句柄！\n"));
		 exception=true;
		 wcsncpy(exceptionmessage,
			L"ObReferenceObjectByHandle failed!\r\n",
			sizeof(L"ObReferenceObjectByHandle failed !\r\n\n")
			);
		 break;
	    }	 
	}
	//********************************获取事件对象成功***********************************
	KdPrint(("start GetOperationInformationFromHashTable:operationtype:%x ",operationtype));
	//开始从hashtable里面获得OperationInformation结构
	POperationInformation pnode=NULL;
	GetOperationInformationFromHashTable(operationtype,&pnode);
	//没有获得这个结构，存储事件对象，等待
	if(pnode==NULL)
	{
		KdPrint(("pnode is null and get the kevent,获得空的节点，储存事件对象在myevents[%d]!\n",number));
		exception=true;
		out->exceptiontype=EXCEPTION_NONE_RECORD;
		wcsncpy(exceptionmessage,
			L"there are no OperationInformation anymore!\r\n",
			sizeof(L"there are no OperationInformation anymore!\r\n\n")
			);
		KeMyEventsLock(number);
		myevents[number]->event=waitevent;
		myevents[number]->waitstatue=true;
		myevents[number]->OperationType=operationtype;
		KeMyEventsUnLock(number);
	
		break;
	}
	KdPrint(("pnode is not null and start write out\n"));
	
	
	 //成功取到了这个结构，那么这时候获得用户程序提供的输出缓冲区来写这些信息
	
	KdPrint(("filename:%wZ filepath:%wZ",pnode->FileName,pnode->FilePath));

	wcsncpy(out->filename,pnode->FileName.Buffer,pnode->FileName.Length);
	wcsncpy(out->filepath,pnode->FilePath.Buffer,pnode->FilePath.Length);
	out->OperationID=pnode->OperationID;
	if(operationtype!=pnode->OperationType)
		KdPrint(("pnode type:%d operationtype:%d",pnode->OperationType,operationtype));
	out->operationtype=pnode->OperationType;
	pnode->wasdealwith=true;                               //设置当前的这个操作信息正在被验证
	::KeSetEvent(waitevent,IO_NO_INCREMENT,false);       //设置事件，通知用户已经取完了信息
	::ObfDereferenceObject(waitevent);                   //减少事件引用
	

	pnode=NULL;                       //释放指针
	KdPrint(("write data over\r\n\n"));
	}while(0);
	//**********如果有意外，写入意外的信息**********
	if(exception)
	wcsncpy(out->exceptionmessage,exceptionmessage,wcslen(exceptionmessage));
	out->exception=exception;
	KdPrint(("GetOperationInformation End!\n"));	
}


//********************************把结果结构体填入哈希表***********************************



//************************************************************************************
/*BOOLEAN GetOperationMessage(PIRP irp)  //METHOD_OUT_DIRECT
{
	NTSTATUS status;
	PIO_STACK_LOCATION irpsp=::IoGetCurrentIrpStackLocation(irp);
	ULONG inlen=irpsp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG outlen=irpsp->Parameters.DeviceIoControl.OutputBufferLength;
	
	HANDLE event=*(HANDLE*)irp->AssociatedIrp.SystemBuffer;
	POperationRecord out=0;

	if(event!=NULL)
	{
		
		PKEVENT kevent;
	
		
		status=ObReferenceObjectByHandle(event,EVENT_MODIFY_STATE,*ExEventObjectType,KernelMode,(PVOID*)&kevent,NULL);
		
		if(NT_SUCCESS(status))
		{
			KdPrint(("get event from application !\n\n"));
			out=(POperationRecord)MmGetSystemAddressForMdlSafe(irp->MdlAddress,NormalPagePriority);
			if(out!=NULL)
			{
				

		        ::KeSetEvent(kevent,IO_NO_INCREMENT,false);
				::ObfDereferenceObject(kevent);
				KdPrint(("write data over!\n\n"));
				return true;
			}
		}
		return false;
	}
	else
	{
		KdPrint(("systembuffer is null in GetOperationMessage!\n\n"));
		return false;
	}
}*/
ULONG GetOperationInformationFromHashTable(ULONG OperationType,OUT POperationInformation* operinfos)
{
	KdPrint(("开始查找操作类型为%d(%x)的操作信息结构！",OperationType,OperationType));
	CHAR message[124];
	unsigned short number;//当前所关心的irp存储在哪个节点下面
	PHASH_TABLE_NODE tablenode;
	ULONG allnodenumber;
	number=HashFunction_ForOperation(OperationType);
	if(number<0 ||number>=HASH_TABLE_LENGTH)//
	{
		strcpy(message,"number is illegal!\n");
		goto ERROR;
		
		return -1;
	}
	KdPrint(("1\n\n"));
	tablenode=&(hashtable.link[number]);
     HashTableNodeListLock(tablenode);	
	 KdPrint(("2\n\n"));
	if(tablenode->OperationType!=OperationType) //如果当前的节点存储的irp类型不符合那么返回
	{
		KdPrint(("3\n\n"));
		KdPrint(("tablenode->OperationType:%d %x OperationType:%d %x number:%d",tablenode->OperationType,tablenode->OperationType,OperationType,OperationType,number));
		strcpy(message,"OperationType mismatch!\n");
		KdPrint(("4\n\n"));
		goto ERROR;
		return -2;
	}
	KdPrint(("5\n\n"));
	allnodenumber=tablenode->number;
	PLIST_ENTRY px=&tablenode->entry;
	KdPrint(("6\n\n"));
	if(allnodenumber==0 )                         //如果当前节点下面没有挂接OperationInformation
	{
		if(!IsListEmpty(px))
		{
			tablenode->number=QueryListNodeCount(px);   //纠正当前的number
		}
		else
		{
		strcpy(message,"list is empty!\n");
		goto ERROR;
			return 0;
		}
	}
	PLIST_ENTRY node;
	node=GetListTailNode(px);               //由于插入的时候是从头插入，这里就需要从尾取出(而且是没有被处理的)，来保持一个队列的形式
	if(node==NULL)
	{

		operinfos=NULL;
		strcpy(message,"node is NULL!\n");
		goto ERROR;
		return -3;
	}
		  ;
	POperationInformation toreturn=CONTAINING_RECORD(node,OperationInformation,listentry);
    if(toreturn!=NULL)
	{
		KdPrint(("successfully\n"));
		*operinfos=toreturn;
		
		
	}
	tablenode->number--;
	allnodenumber=tablenode->number;

	
	HashTableNodeListUnLock(tablenode);
	
	return allnodenumber;

ERROR:
	KdPrint(("errormessage:%s",message));
	HashTableNodeListUnLock(tablenode);
	return 0;
}
//**********************用来查找具有所给参数特性的OperationResult并返回************************

PLIST_ENTRY GetListTailNode(PLIST_ENTRY px)
{
	if(IsListEmpty(px))
	{
		KdPrint(("empty\n"));
		return 0;
	}
	PLIST_ENTRY p,head;
	head=px;
	
	ULONG i=0;
	
	int m=0;
	p=head->Blink;  //尾节点
    while(p!=head)
	{
		POperationInformation po=(POperationInformation)p;
			if(po->wasdealwith!=true)             //如果这个结构没有被处理
				return p;
	    p=p->Blink;
		m++;
	}
	KdPrint(("not found :%d",m));
	return NULL;
}
ULONG QueryListNodeCount(PLIST_ENTRY px)
{
	if(IsListEmpty(px))
		return 0;
	PLIST_ENTRY p,head;
	head=px;
	ULONG i=0;
	
	for(p=head->Flink;p!=head;p=p->Flink)
	{
		i++;
	}
	
	return i;
	
}
/*ULONG QueryHashTable(IN ULONG type,IN ULONG operationID,OUT OperationInformation operinfos)
{
	UNREFERENCED_PARAMETER(operinfos);
	UNREFERENCED_PARAMETER(operationID);
	unsigned short number;//当前所关心的irp存储在哪个节点下面
	PHASH_TABLE_NODE tablenode;
	ULONG allnodenumber;
	number=HashFunction_ForOperation(type);
	if(number<0 ||number>=HASH_TABLE_LENGTH)//
		return -1;
	tablenode=&(hashtable.link[number]);
	if(tablenode->OperationType!=type) //如果当前的节点存储的irp类型不符合那么返回
		return -2;
	allnodenumber=tablenode->number;
	if(allnodenumber==0)                         //如果当前节点下面没有挂接OperationInformation
	{
		return 0;
	}
	PLIST_ENTRY p,head;
	head=&tablenode->entry;
	POperationInformation node;
	
	for(p=head->Flink;p!=head;p=p->Flink)
	{
	    node=(POperationInformation)p; //由于OperationInformation结构设计成了第一个成员就是PLIST_ENTRY，了PLIST_ENTRY的指针就相当于结构体首地址指针
	    
	}

}*/

VOID InitOperationInformation(POperationInformation infor)
{
	if(infor==NULL)
		return;
	::KeInitializeEvent(&(infor->finished),NotificationEvent,FALSE);
	infor->OperationID=0;
	infor->OperationType=-1;
	infor->wasdealwith=false; 
}

/****************插入之后需要检查myevents中所对应的当前的irp类型的事件是否为空，
                      不为空，那么就需要设置该事件以通知引用程序*****************************/
ULONG FillOperationInformationAndAddToList(ULONG IrpType,POperationInformation operationinformation,PVOID pContext)
{
	
	
	UNREFERENCED_PARAMETER(pContext);
	KIRQL irql;   	
	ULONG ID;
    
	irql=::KeGetCurrentIrql();
	if(irql<=DISPATCH_LEVEL)
	{
	switch(IrpType)
	{
	case(IRP_MJ_CREATE):
		
		break;
    case(IRP_MJ_SET_INFORMATION):
		                        
		KdPrint(("开始添加操作信息！\n"));		  //添加进之后就等待应用程序来取这个并判断返回之后继续进行
		ID=MyOperationInformationAddToHashTable(operationinformation);
		
		break;
	default:
		break;
	}
	}
	return ID;
}

//***************************关于hashtable_result的操作函数********************


//添加操作结构到hashtable_result，等待取走

//添加操作信息到哈希表里面，然后等待用户程序的取
ULONG MyOperationInformationAddToHashTable(POperationInformation operation)
{
	ULONG re=0;
	
	if(operation==NULL)
	{
		KdPrint(("OperationInformation is Valid!\n"));
		return re;
	}
	
		
	ULONG operationtype=operation->OperationType;
	if(operationtype>30 ||operationtype <0 ||operationtype ==30)
	{
		KdPrint(("HashFunction error:%d",operationtype));
		return re;
	}
	ULONG number=HashFunction_ForOperation(operationtype);
	PHASH_TABLE_NODE hashnode=&(hashtable.link[number]);
	PLIST_ENTRY listentryofhashtable=&(hashnode->entry);

	//POperationInformation operationinformation=(POperationInformation)::ExAllocatePool(NonPagedPool,sizeof(OperationInformation));
	    KIRQL irql=::KeGetCurrentIrql();
		if(irql>=DISPATCH_LEVEL)
		KdPrint(("current irql in addtohashtable:%c",irql));

	    KdPrint(("operation filename:%wZ\n",&operation->FileName));
		KdPrint(("attemp to access node->number:%d init:%din function  MyOperationInformationAddToHashTabl!",hashnode->number,hashnode->initilzed?1:0));
		HashTableNodeListLock(hashnode);
		::InsertHeadList(&hashnode->entry,(PLIST_ENTRY)operation);
		if(hashnode->number==0 &&hashnode->totalnumbers==0)
		{
			hashnode->OperationType=operation->OperationType;
		}
		hashnode->number++;
		hashnode->totalnumbers++;
		re=operation->OperationType+hashnode->totalnumbers;
		operation->OperationID=re;
		KdPrint(("添加完毕:新节点在link[%d],OperationType:%x ID:%d \n开始查询是否有线程在等待\n",number,hashnode->OperationType,re));
		KeMyEventsLock(number);
		
		if(myevents[number]->OperationType==operation->OperationType &&myevents[number]->waitstatue && myevents[number]->event!=NULL)
		{
			
			LONG singled;
			
			singled=::KeSetEvent(myevents[number]->event,IO_NO_INCREMENT,FALSE);
		    ObfDereferenceObject(myevents[number]->event);
			if(singled==0)          //如果已经是受信状态会返回非零值
			{	
			KdPrint(("通知应用程序！\n\n"));
			myevents[number]->waitstatue=false;
			}
			else
			{
				KdPrint(("KeSetEvent:event 已经是授信状态！\n\n"));//句柄是当前进程有效的，所以
				                                     //保存句柄是错误的做法,应该保存内核对象
			}
			myevents[number]->event=NULL;
		}
		else
		{

			KdPrint(("当前节点没有事件在等待:myevents[%d].operationtype:%d operationtype:%d waitstatu: %d!\n",number,myevents[number]->OperationType,operationtype,myevents[number]->waitstatue?1:0));
		}
		 KeMyEventsUnLock(number);
		HashTableNodeListUnLock(hashnode);
		
		//::ExInterlockedInsertHeadList(listentryofhashtable,(PLIST_ENTRY)&operation->listentry,spn); 
		return re;
}
//*************************//
