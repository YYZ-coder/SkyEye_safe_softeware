#include "stdafx.h"


extern NPAGED_LOOKASIDE_LIST looksidelist; 
extern NPAGED_LOOKASIDE_LIST looksidelist_result;
extern HASH_TABLE hashtable; 
extern HASH_TABLE hashtable_result;
extern ThreadEvents myevents[HASH_TABLE_LENGTH];


extern POperationResult resultfortest;
extern size_t process_name_offset;

/*集成整个控制流程，提供一个简洁的调用接口
   整个控制流程大概如下：在当前线程收集信息，
   然后在链表里查询当前的文件是否为关心的文件
   如果是，填写，然后等待应用程序的返回最后根
   据结果完成相应的处理，否则按照默认的逻辑处理 */
NTSTATUS VerifyControl(POperationInformation operationinformation,POperationResult result)
{
	        int flag=0;
	        ULONG ID;
			NTSTATUS statu;
		/*	KIRQL oldirqls;
		KeAcquireSpinLock(&test_lock,&oldirqls);
		KdPrint((" in lock!\n"));
		if((intest!=0))
		{
			KdPrint(("直接返回！\n"));
			KeReleaseSpinLock(&test_lock,oldirqls);
			statu=STATUS_ACCESS_DENIED;
			irp->IoStatus.Information=0;
			irp->IoStatus.Status=statu;
			IoCompleteRequest(irp,IO_NO_INCREMENT);
			return statu;
		}
		KdPrint(("leave lock\n"));
		 intest++;
		KeReleaseSpinLock(&test_lock,oldirqls);*/
		  //  InitOperationInformation(operationinformation);
			//operationinformation->finished=event;
			// InitOperationResult(result);
			//::RtlInitUnicodeString(&(operationinformation->FileName),(PCWSTR)directory->Buffer);

			//::RtlInitUnicodeString(&(operationinformation->FilePath),(PCWSTR)directory->Buffer);
			//operationinformation->OperationType=IRP_MJ_SET_INFORMATION;
			//result->operationtype=IRP_MJ_SET_INFORMATION;

			KdPrint(("插入OperationInformation等待验证%d!\n",::KeGetCurrentIrql()));
			
			ID=::FillOperationInformationAndAddToList(IRP_MJ_SET_INFORMATION,operationinformation,NULL);
			result->operationID=ID;
			result->wasFill=false;
			wcsncpy(result->otherinfo,L"flag --\r\n",wcslen(L"flag --\r\n\n"));
			::FillOperationResultAddToHashTable_Result(result);
			KdPrint(("&&&&&&&&&&&&&&&&&&&&&&&&&&&wait for current irql:%d",KeGetCurrentIrql()));
			statu=KeWaitForSingleObject(&(operationinformation->finished), Executive, KernelMode, FALSE, NULL);
			
			KdPrint((" event current irql:%d",::KeGetCurrentIrql()));
			bool permit=false;
			
			if(result!=NULL &&result->wasFill)
			{
			KdPrint(("在哈希表中取得验证结果!:%d释放内存\n",result->operation_permit?1:0));
			permit=result->operation_permit;
			::ExFreeToNPagedLookasideList(&looksidelist_result,result);
			}
			else
			KdPrint(("发生意外：取得一个空的结果!\n\n"));

			::ExFreeToNPagedLookasideList(&looksidelist,operationinformation);  //把hashtable里面的对应的operationinformation结构从链表里面释放出来
			if(permit)
			{
			KdPrint(("操作被拒绝! %wZ\n",&operationinformation->FileName));
			statu=STATUS_ACCESS_DENIED;
			return statu;
			}
			else
			{
				statu=STATUS_SUCCESS;
				KdPrint(("操作被允许！%wZ\n",&operationinformation->FileName));
			}
			return statu;
};
VOID     AllocatePool(OUT POperationInformation &operationinformation,OUT POperationResult &result)
{
	KdPrint(("AllocarePool In!\n\n"));
	operationinformation=(POperationInformation)::ExAllocateFromNPagedLookasideList(&looksidelist);
	result=(POperationResult)::ExAllocateFromNPagedLookasideList(&looksidelist_result);

    if(operationinformation==NULL || result==NULL)
	{
		KdPrint(("allocate from lookaside is failed!\n"));
		operationinformation=NULL;
		result=NULL;
	}
	InitOperationInformation(operationinformation);
			
	InitOperationResult(result);

}

