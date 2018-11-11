// sfilterApp.cpp : Defines the entry point for the console application.

#include "stdafx.h"


extern MYSQL mysql;
bool flag=false;
void Init(POperationRecord a)
{

}

int main(int argc, char * argv[])
{
	 mysql_init(&mysql);
	printf("version:1.36\n %d \n%d \n %ws",IOCTL_VERIFY_OPERATION_INFORMATION,IOCTL_SEND_VERIFY_RESULT,L"asdaeda");
	
	HANDLE hfile=::CreateFileA("\\\\.\\SFilterDevice",GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_SYSTEM,NULL);

	if(hfile==INVALID_HANDLE_VALUE)
	{
		 hfile=CreateFileA("////.//SFilterDevice",GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_SYSTEM,NULL);
		if(hfile==INVALID_HANDLE_VALUE)
		printf("无效的设备句柄，打开设备失败:%d\n",::GetLastError());
		system("pause");
		return -1;
	}
	

	HANDLE hthread2=CreateThreadDefault((LPVOID)hfile,init_carefile);

	::WaitForSingleObject(hthread2,INFINITE);

	HANDLE hthread=CreateThreadDefault((LPVOID)hfile,threadfun);
	char  flag[124];
	while(1)
	{
	gets_s(flag);
	if(strcmp(flag,"exit")==0)
		return 0;
	}
    system("pause");
	CloseHandle(hfile); 
	CloseHandle(hthread);
	CloseHandle(hthread2);
	return 0;
}	
                                                                        