
#include "ProcessForbid.h"

extern LIST_ENTRY ForbidProcessList;

extern KSPIN_LOCK ForbidProcessListLock;

//插入指定进程名进入进程进程链
BOOLEAN InsertForbidProcessList(PForbidProcess pFP){
	BOOLEAN inserted = FALSE;
	KLOCK_QUEUE_HANDLE handle;
	KeAcquireInStackQueuedSpinLock(&ForbidProcessListLock, &handle);

	InsertHeadList(&ForbidProcessList, &pFP->listEntry);
	inserted = TRUE;

	KeReleaseInStackQueuedSpinLock(&handle);
	return inserted;
}

//移除指定进程名
BOOLEAN RemoveForbidProcessList(PForbidProcess pFP){
	BOOLEAN removed = FALSE;
	KLOCK_QUEUE_HANDLE handle;
	KeAcquireInStackQueuedSpinLock(&ForbidProcessListLock, &handle);

	RemoveEntryList(&pFP->listEntry);
	removed = TRUE;

	KeReleaseInStackQueuedSpinLock(&handle);
	return removed;
}
