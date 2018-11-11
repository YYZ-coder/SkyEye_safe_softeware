#include "stdafx.h"

DWORD WINAPI threadfun(LPVOID para)
{
  BOOL iResult;
  HANDLE myevent;    //这个传入给内核，用于获得可以取操作信息的通知
  time_t now=time(NULL);//现在的时间

  time_t times=0;;
 
	printf("thread start:%d",(int)para);
	HANDLE hdevice=(HANDLE)para;
	OperationRecord zrecord;
	OperationResult zresult;
	ZeroMemory(&zrecord,sizeof(OperationRecord));
	ZeroMemory(&zresult,sizeof(OperationResult));

	POperationRecord record=&zrecord;
	POperationResult result=&zresult;
	if(para==NULL || record==NULL ||result==NULL)
	{
		printf("Paramter Is Valid!");
		return 0;
	}
	else
	{
		while(true)   
		{
		 
		//nows=time(NULL); //现在的时间
		//if(nows-now<=3)//过了3秒
			//continue;
		printf("start wait for get operation information from kernel: the %dth times\r\n",++times);
		if(hdevice==INVALID_HANDLE_VALUE)
			printf("invalid handle!");
		HANDLE event =CreateEvent(NULL,FALSE,FALSE,NULL);
		HANDLE forback=CreateEvent(NULL,FALSE,FALSE,NULL);
		myevent=CreateEvent(NULL,FALSE,FALSE,NULL);
		if(event==NULL)
		{
			printf("create event failed!");
			return -1;
		}
		//wcsncpy(record->filepath,L"",strlen(""));

		record->event=event;
		record->exception=FALSE;
		record->forback=forback;
		record->operationtype=0x06;//IRP_MJ_SET_INFORMATION
		DWORD sizeforreturn;
		//通知内核取出操作信息
		printf("IOCTL_VERIFY_OPERATION_INFORMATION:%x  %d---\n",record->operationtype,record->operationtype);
		iResult=::DeviceIoControl(hdevice,IOCTL_VERIFY_OPERATION_INFORMATION,&myevent,sizeof(HANDLE),record,sizeof(OperationRecord),&sizeforreturn,NULL);
		
		printf("waitting for return.........\n");
		
		::WaitForSingleObject(myevent,INFINITE);
		printf("havev getten the operation infromation:\n");

		if(iResult)
		{
			if(record->exception)
			{
				
				if(record->exceptiontype==EXCEPTION_NONE_RECORD) //如果没有记录，那么直接跳过当前的循环
				continue;
				else
				{
					printf("there have exception in getting operation information:%s",record->exceptionmessage);
					break;
				}
			}
			printf("---recive operation record information from kernel---\n");
			printf("operation type:%d( %x) \n operation object:%ws \n operation \nother information: %ws\n"
				,record->exceptiontype,
				record->exceptiontype,
				record->filename,				
				record->otherinfo);
		}
		else
		{
			printf("\nGet Operation Information From kernel Failed!\n");
			break;
		}
		 //至此已经取出需要验证的操作信息，这里睡眠0.5秒来模仿整个验证过程
		 printf("verfiying the operation information!，permit the operation?(Y/N)\n");
		 //Sleep(500);

		 result->operation_permit=true;
		 result->operationtype=0x06;
		 result->operationID=record->OperationID;
		 printf("IOCTL_SEND_VERIFY_RESULT\n:OperationType:%d",result->operationtype);
		 iResult=::DeviceIoControl(hdevice,IOCTL_SEND_VERIFY_RESULT,result,sizeof(OperationResult),NULL,0,&sizeforreturn,NULL);
		 if(iResult)
			 printf("已近完成了此次操作验证！");
		 else
			 printf("未能完成此次操作验证!");

		//::SetEvent(forback);通知内核操作已经完成
		 
		

		CloseHandle(event);
		CloseHandle(forback);
		printf("have finished once operation verify!");
		now=time(NULL);
	
		}
	}
	return 0;
}
DWORD WINAPI init_carefile(LPVOID para)
{
	printf("开始等待初始化关心文件链表\n");

	HANDLE hfile=(HANDLE)para;
	CareFile_T carefilenode;
	ZeroMemory(&carefilenode,sizeof(CareFile_T));
	int number=0;

	HANDLE waitevent;
	waitevent=CreateEvent(NULL,FALSE,FALSE,NULL);
	if(waitevent==INVALID_HANDLE_VALUE)
	{
		printf("创建事件失败(init_carefile)!\n");
	}
	BOOLEAN INIT=TRUE;;
	BOOLEAN result;
	DWORD returnsize;
	DWORD WAITSTATU; 

	result=::DeviceIoControl(hfile,
		IOCTL_CAREFILE_INIT,
		&waitevent,
		sizeof(HANDLE),
		&INIT,
		sizeof(BOOLEAN),
		&returnsize,
		NULL);	

	WaitForSingleObject(waitevent,INFINITE);
	
	if(result)
    {
		printf("收到内核消息：init:%d",INIT?1:0);
		if(INIT)        //如果内核已经初始化完毕了，直接返回
			return 0;
	}

	
	while(!INIT && number<3)  //如果内核需要初始化
	{
		HANDLE events;
		events=CreateEvent(NULL,FALSE,FALSE,NULL);
	CareFileTransfer transfer;
	ZeroMemory(&transfer,sizeof(CareFileTransfer));
	transfer.number=EACH_NUMBER;
	PCareFile_T temp=transfer.filenode;

	ZeroMemory(&transfer,sizeof(CareFileTransfer));
	for(int i=0;i<transfer.number;i++)
	{
		wcsncpy_s(temp[i].filename,L"1234.txt",wcslen(L"1234.txt"));
		wcsncpy_s(temp[i].filename,L"c:\\windows\\1234.txt",wcslen(L"c:\\windows\\1234.txt"));
		temp[i].Owner=12;;
		temp[i].secutiry_level=1;
	}

	

	printf("开始向内核发送文件数据:\n");

	result=::DeviceIoControl(hfile,
		IOCTL_CAREFILE_START_INIT,
		&events,
		sizeof(HANDLE),
		&transfer,
		sizeof(CareFileTransfer),
		&returnsize,
		NULL);

	WAITSTATU=::WaitForSingleObject(events,INFINITE);
	if(!result)
	{
		printf("发送失败!\n");
		break;
	}
	
	number++;
	printf("已经发送了%d条文件记录!\n",number);
	
	}
	HANDLE finished=CreateEvent(NULL,FALSE,FALSE,NULL);
	result=::DeviceIoControl(hfile,
		IOCTL_CAREFILE_INIT_SHUTDOWN,
		&finished,
		sizeof(HANDLE),
		NULL,
		NULL,
		&returnsize,
		NULL);
	WaitForSingleObject(finished,INFINITE);
	printf("初始化完毕!\n");
	return 0;
}


HANDLE CreateThreadDefault(LPVOID para,LPTHREAD_START_ROUTINE oi)
{
	DWORD threadid;
	
	HANDLE Thread1=(HANDLE)CreateThread(NULL,0,oi,para,0,&threadid);
	return Thread1;
}