#include "stdafx.h"



//****************sfilter.cpp*********************

extern NPAGED_LOOKASIDE_LIST looksidelist;
extern NPAGED_LOOKASIDE_LIST looksidelist_result;
extern HASH_TABLE hashtable; 
extern HASH_TABLE hashtable_result;
extern PThreadEvents myevents[HASH_TABLE_LENGTH];

extern POperationResult resultfortest;
//**************************************************************************************

BOOLEAN FillOperationResultAddToHashTable_Result(POperationResult operationresult);

PVOID SetOperationResultFromHashTable_Result(ULONG OperationType,ULONG ID,POperationResult result);

VOID GetOperationResult(POperationResult result,ULONG type,ULONG ID);
PLIST_ENTRY GetTheResultNode(PLIST_ENTRY px,ULONG ID);
VOID SetOperationResult(PIRP irp);
VOID MyOperationResultAddToHashTable2(POperationResult result);

BOOLEAN CutOffListNode(ULONG ID,PLIST_ENTRY px);         //使OperationResult脱离链表


BOOLEAN CutOffListNode(ULONG ID,PLIST_ENTRY px)
{
	KdPrint(("正在删除指定的OPerationResult节点!\n\n"));
	PLIST_ENTRY head,temp;
	head=px;
	temp=head->Blink;
	for(temp;temp!=head;temp=temp->Blink)
	{
		POperationResult result=(POperationResult)CONTAINING_RECORD(temp,OperationResult,entry);
		if(result->operationID==ID)
		{
			PLIST_ENTRY flink,blink;
			flink=temp->Blink;
			blink=temp->Flink;
			flink->Flink=blink;
			blink->Blink=flink;
			KdPrint(("成功删除!\n"));
			return TRUE;
			
		}
	}
	return FALSE;
}

VOID  InitOperationResult(POperationResult re)
{
	if(re==NULL)
		return;
	re->wasFill=false;
	re->operation_permit=0;
	re->operationID=0;
	re->operationtype=0;
	RtlZeroMemory(re->otherinfo,sizeof(WCHAR)*512);
}
VOID SetOperationResult(PIRP irp)  //把从app获得的验证结果插入到hashtable_result中并且通知当前等待的事件
{
	KIRQL oldirql=::KeGetCurrentIrql();
	KIRQL now=oldirql;
	KdPrint(("SetOperationResult:current irql:%d",oldirql));
	PIO_STACK_LOCATION irpsp=::IoGetCurrentIrpStackLocation(irp);
	PVOID buffer=irp->AssociatedIrp.SystemBuffer;
	POperationResult result;
	POperationResult newresult;




	result=(POperationResult)buffer;
	if(result==NULL)
		return;
	KdPrint(("SetOperationResult:current irql:%d",now));

	KdPrint(("查找对应的OperationResult结构！\n\n"));
	newresult=(POperationResult)SetOperationResultFromHashTable_Result(result->operationtype,result->operationID,result);
	KdPrint(("SetOperationResult:current irql:%d\n",now));
	if(newresult==NULL)
	{
		KdPrint(("空的OperationResult结构！\n\n"));
		return ;
	}
	KeRaiseIrql(DISPATCH_LEVEL,&oldirql);

	KdPrint(("赋值前的值:\n------ID:%x\n------ operationtype:%x(%d)(%d)\n------permit:%d otherinfo %ws\n",
		newresult->operationID,newresult->operationtype,newresult->operationtype,result->operationtype,newresult->operation_permit?1:0,newresult->otherinfo));
	KdPrint(("从应用获得验证信息如下:\n------ID:%x\n------ operationtype:%x(%d)(%d) \n------permit:%d\n",
		result->operationID,result->operationtype,result->operationtype,result->operationtype,result->operation_permit?1:0));

	KdPrint(("赋值后信息如下:\n------ID:%x\n------ operationtype:%x(%d) \n------permit:%d\n wasfill:%d\n",
		newresult->operationID,newresult->operationtype,newresult->operationtype,newresult->operation_permit?1:0,newresult->wasFill?1:0));
	
	//把获得新的结果插入到哈希比表，然后检查对应的OperationInformation并且把它释放掉
	 MyOperationResultAddToHashTable2(newresult);

	PHASH_TABLE_NODE tablenode;
	ULONG number=HashFunction_ForOperation(result->operationtype);
	tablenode=&(hashtable.link[number]);
	 	PLIST_ENTRY p,head;
	POperationInformation results;
	head=&tablenode->entry;
	::HashTableNodeListLock(tablenode);
		 	now=::KeGetCurrentIrql();
	KdPrint(("SetOperationResult:current irql:%d\n",now));
	for(p=head->Flink;p!=head;p=p->Flink)        
	{
        results=(POperationInformation)p;
		if(newresult->operationID==results->OperationID)
		{
			KdPrint(("\n成功的找到对应的OperationInformation结构\n\n"));
			//DbgBreakPoint();
			PLIST_ENTRY flink,blink;
			flink=p->Blink;
			blink=p->Flink;
			flink->Flink=blink;
			blink->Blink=flink;   //使当前的操作信息结构脱离hashtable,方便后面直接释放这个内存
			::KeSetEvent(&(results->finished),IO_NO_INCREMENT,FALSE);//通知对应的线程该操作已经被验证
			KdPrint(("kesetevent\n\n"));	
			break;
		}
	}

	::HashTableNodeListUnLock(tablenode);
	now=::KeGetCurrentIrql();
	KdPrint(("current irql:%d\n",now));
	if(oldirql!=now)
	{
		KdPrint(("irql change!\n\n"));
	            //恢复到原来的IRQL等级，否则会出现IRQL_GT_ZERO_AT_SYSTEM_SERVICE的蓝屏错误
		
	}	
	KeLowerIrql(oldirql);
	ASSERT(KeGetCurrentIrql()==PASSIVE_LEVEL);
	KdPrint(("leave from SetOperaitonResult\n\n"));
}
VOID GetOperationResult(POperationResult result,ULONG type,ULONG ID)//
{
	ULONG number=HashFunction_ForOperation(type);
	PHASH_TABLE_NODE tablenode;
	KdPrint(("GetOperationResult!\n\n"));
	tablenode=&(hashtable_result.link[number]);
	if(tablenode->OperationType!=type)
	{
		KdPrint(("发生错误在比对IRP_TYPE时%d --%d(GetOperationResult)",tablenode->OperationType,type));
		return;
	}
	PLIST_ENTRY p,head;
	POperationResult results;
	head=&tablenode->entry;
	::HashTableNodeListLock(tablenode);
	for(p=head->Flink;p!=head;p=p->Flink)        
	{
        results=(POperationResult)p;
		if(results->operationID==ID)
		{
			result=results;
			break;
		}
	}
	::HashTableNodeListUnLock(tablenode);
	
}
BOOLEAN FillOperationResultAddToHashTable_Result(POperationResult operationresult)
{
	if(operationresult==NULL)
		return FALSE;
	if(operationresult->operationID==0 || operationresult->operationtype==0)
		return FALSE;
	KdPrint(("插入空的结果集并等待程序填写:%d!\n",::KeGetCurrentIrql()));
	 MyOperationResultAddToHashTable2(operationresult);
	 KdPrint(("插入完毕!%d\n",::KeGetCurrentIrql()));
	 return true;
}
PVOID SetOperationResultFromHashTable_Result(ULONG OperationType,ULONG ID,POperationResult result)
{
	KdPrint(("开始查找操作类型为%d(%x) ID为：%d 的OperationResult 结构！",OperationType,OperationType,OperationType,ID));
	CHAR message[124];
	unsigned short number;//当前所关心的irp存储在哪个节点下面
	PHASH_TABLE_NODE tablenode;
	ULONG allnodenumber;
	number=HashFunction_ForOperation(OperationType);
	if(result==NULL)
	{
		strcpy(message,"POperationResult is NULL in function SetOperationResultFromHashTable_Result!\n\n");
		goto ERROR;
		
		return NULL;
	}
	if(number<0 ||number>=HASH_TABLE_LENGTH)//
	{
		strcpy(message,"number is illegal!\n");
		goto ERROR;
		
		return NULL;
	}
	tablenode=&(hashtable_result.link[number]);
    HashTableNodeListLock(tablenode);	

	if(tablenode->OperationType!=OperationType) //如果当前的节点存储的irp类型不符合那么返回
	{
		KdPrint(("tablenode->OperationType:%d %x OperationType:%d %x number:%d",tablenode->OperationType,tablenode->OperationType,OperationType,OperationType,number));
		strcpy(message,"OperationType mismatch!\n");
		goto ERROR;
		return NULL;
	}
	
	allnodenumber=tablenode->number;
	PLIST_ENTRY px=&tablenode->entry;
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
			return NULL;
		}
	}
	PLIST_ENTRY node;
	node=GetTheResultNode(px,ID);       //
	if(node==NULL)
	{
		
		strcpy(message,"node is NULL!\n");
		goto ERROR;
		return NULL;
	}
		  ;
	POperationResult toreturn=CONTAINING_RECORD(node,OperationResult,entry);
    if(toreturn!=NULL)
	{
		KdPrint(("成功找到该节点successfully ,:wasfill:%d\n otherinfo:%ws",toreturn->wasFill?1:0,toreturn->otherinfo));
		toreturn->wasFill=true;
		toreturn->operation_permit=result->operation_permit;
		wcsncpy(toreturn->otherinfo,result->otherinfo,sizeof(WCHAR)*wcslen(result->otherinfo));
		KdPrint(("填写完成: wasfill:%d\n otherinfo:%ws ",toreturn->wasFill?1:0,toreturn->otherinfo));
	}
	CutOffListNode(ID,px);
	tablenode->number--;
	allnodenumber=tablenode->number;

	
	HashTableNodeListUnLock(tablenode);
	
	return toreturn;

ERROR:
	KdPrint(("errormessage:%s",message));
	HashTableNodeListUnLock(tablenode);
	return 0;
}
VOID MyOperationResultAddToHashTable2(POperationResult result)
{
	KdPrint(("MyOperationResultAddToHashTable2:\n\n"));
	if(result==NULL)
	{
		KdPrint(("添加了一个空的OperationResult！\n"));
		return;
	}
	ULONG id=result->operationID;//需要初步验证这个ID
	if(id==0 ||id <0)
	{
		KdPrint(("不合法的ID！\n\n"));
	}
	
	
	ULONG operationtype=result->operationtype;
	ULONG number=HashFunction_ForOperation(operationtype);
	if(operationtype>30 ||operationtype <0 ||operationtype ==30)
	{
		KdPrint(("HashFunction error:%d",operationtype));
		return ;
	}

	PHASH_TABLE_NODE hashnode=&(hashtable_result.link[number]);
	PLIST_ENTRY listentryofhashtable=&(hashnode->entry);

	//POperationInformation operationinformation=(POperationInformation)::ExAllocatePool(NonPagedPool,sizeof(OperationInformation));
	    KIRQL irql=::KeGetCurrentIrql();
		if(irql>=DISPATCH_LEVEL)
		KdPrint(("current irql in addtohashtable:%c",irql));

	//    KdPrint(("operation filename:%wZ\n",&operation->FileName));
	//	KdPrint(("attemp to access node->number:%d init:%din function  MyOperationInformationAddToHashTabl!",hashnode->number,hashnode->initilzed?1:0));
		HashTableNodeListLock(hashnode);
		::InsertHeadList(&hashnode->entry,(PLIST_ENTRY)result);

		
		if(hashnode->number==0)
		{
			hashnode->OperationType=result->operationtype;
		}
		hashnode->number++;
		HashTableNodeListUnLock(hashnode);
		KdPrint(("MyOperationResultAddToHashTable2 end!\n\n"));
}
PLIST_ENTRY GetTheResultNode(PLIST_ENTRY px,ULONG ID)
{
	KdPrint(("正在查找指定的ID\n\n"));
	PLIST_ENTRY head,temp;
	head=px;
	temp=head->Blink;
	for(temp;temp!=head;temp=temp->Blink)
	{
		POperationResult result=(POperationResult)CONTAINING_RECORD(temp,OperationResult,entry);
		if(result->operationID==ID)
		{
		//	KdPrint(("找到了指定的id %d!",result->operationID));
			//KdPrint(("试着改变其值:wasfill:%d",result->wasFill?1:0));
			//result->wasFill=true;
			//KdPrint(("改变之后:wasfill:%d",result->wasFill?1:0));
			return temp;
			
		}
	}
	return NULL;
}