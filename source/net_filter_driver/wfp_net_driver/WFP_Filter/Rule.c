#include "FilterLayers.h"
#include <ntddk.h>
#include <wdm.h>

extern LIST_ENTRY RuleList;

extern KSPIN_LOCK  RuleListLock;

VOID WfpRuleListInit(){
	
}

VOID WfpRuleListUnInit(){
	KdPrint(("Rule List UnInit\n"));
	KLOCK_QUEUE_HANDLE handle;
	KeAcquireInStackQueuedSpinLock(&RuleListLock,&handle);
	if (!IsListEmpty(&RuleList)){
		while (!IsListEmpty(&RuleList)){
			PLIST_ENTRY pEntry = RemoveTailList(&RuleList);
			PRULES_LIST data = CONTAINING_RECORD(pEntry, RULES_LIST, listEntry);
			ExFreePool(data);
		}
	}
	KeReleaseInStackQueuedSpinLock(&handle);
}

//添加Rule
NTSTATUS AddRuleFromR3(PRULE_FROM_R3 pRFR){
	NTSTATUS Status = STATUS_SUCCESS;

	PRULES_LIST p = ExAllocatePool(NonPagedPool, sizeof(RULES_LIST));
	RtlZeroMemory(p, sizeof(RULES_LIST));

	int sizeIN = strlen(pRFR->ImageName);
	int sizePP = wcslen(pRFR->ProcessPath);

	for (int i = 0; i < sizeIN; i++){
		p->Rule.ImageName[i] = pRFR->ImageName[i];
	}

	for (int j = 0; j < sizePP; j++){
		p->Rule.ProcessPath[j] = pRFR->ProcessPath[j];
	}

	Status = InsertRuleList(p);
	if (Status == STATUS_UNSUCCESSFUL){
		KdPrint(("Insert Failed!\n"));
	}
	else{
		KdPrint(("Insert Success!\n"));
	}

	return Status;
}

//插入到RuleList
NTSTATUS InsertRuleList(PRULES_LIST rl){
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	KLOCK_QUEUE_HANDLE LockHandle;

	KeAcquireInStackQueuedSpinLock(&RuleListLock, &LockHandle);

	KdPrint(("Insert Rule List\n"));

	InsertHeadList(&RuleList, &rl->listEntry);
	Status = STATUS_SUCCESS;

	KeReleaseInStackQueuedSpinLock(&LockHandle);
	return Status;
}

//移除指定的Rule从RuleList
NTSTATUS RemoveRule(PRULE_FROM_R3 rl){
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	KdPrint(("Remove Rule List\n"));
	KLOCK_QUEUE_HANDLE lockHandle;
	KeAcquireInStackQueuedSpinLock(&RuleListLock,&lockHandle);

	LIST_ENTRY* entry = NULL;
	PRULES_LIST p = NULL;
	int time = 1;

	if (IsListEmpty(&RuleList)){
		KdPrint(("Rule List is Empty!!!\n"));
	}
	else{
		for (entry = RuleList.Flink;
			entry != &RuleList;
			entry = entry->Flink){
			//先遍历整个链表，并且确定是哪个节点
			int Sa = 0;
			int Pp = 0;
			
			PRULES_LIST prl = CONTAINING_RECORD(entry, RULES_LIST, listEntry);
			//ImageName匹配
			for (int i = 0; i < strlen(rl->ImageName); i++){
				if (prl->Rule.ImageName[i] == rl->ImageName[i])
					Sa += 1;
			}
			//ProcessPath匹配
			for (int j = 0; j < wcslen(rl->ProcessPath); j++){
				if (prl->Rule.ProcessPath[j] == rl->ProcessPath[j])
					Pp += 1;
			}
			if (Sa == strlen(rl->ImageName)){
				RemoveEntryList(&prl->listEntry);
				Status = STATUS_SUCCESS;
				break;
			}
			if (Pp == wcslen(rl->ProcessPath)){
				//KdPrint(("ProcessPath 匹配规则节点成功！！！\n"));
			}
			time += 1;
		}
	}
	
	KeReleaseInStackQueuedSpinLock(&lockHandle);

	return Status;
}

//数据过滤：匹配成功，返回STATUS_SUCCESS;匹配不成功，返回STATUS_UNSUCCESSFUL
NTSTATUS Data_Filtering(PBIND_DATA_LIST pBDL){
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	LIST_ENTRY* entry = NULL;

	if (IsListEmpty(&RuleList)) KdPrint(("RuleList is Empty\n"));
	else{
		int t = 1;
		//遍历RuleList
		for (entry = RuleList.Flink;
			entry != &RuleList;
			entry = entry->Flink){

			//KdPrint(("第%d个规则匹配！！！！",t));

			PRULES_LIST prl = CONTAINING_RECORD(entry, RULES_LIST, listEntry);
			int sizeIN = strlen(prl->Rule.ImageName);
			int sizePP = wcslen(prl->Rule.ProcessPath);
			int PPs = 0;
			int INs = 0;

			//匹配Rule中的ImageName
			for (int i = 0; i < sizeIN; i++){
				if (prl->Rule.ImageName[i] == pBDL->imageName[i]){
					INs += 1;
				}
			}
			//KdPrint(("INs:%d\nsizeIN:%d\n",INs,sizeIN));
			if (INs == sizeIN){
				return STATUS_SUCCESS;
			}

			//匹配Rule中的ProcessPath
			for (int j = 0; j < sizePP; j++){
				if (prl->Rule.ProcessPath[j] == pBDL->wProcessPath[j])
					PPs += 1;
			}
			if (PPs == sizePP){
				return STATUS_SUCCESS;
			}
			t+=1;
		}
	}
	
	return Status;
}
