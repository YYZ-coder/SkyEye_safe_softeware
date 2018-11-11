#include "stdafx.h"


extern NPAGED_LOOKASIDE_LIST carefile_verify;

extern UserInfo user_info;
NTSTATUS  DBAC_ControlLogic(PUNICODE_STRING filepath,ULONG irp_type)
{
	NTSTATUS status=STATUS_ACCESS_DENIED;
//	UNREFERENCED_PARAMETER(filename);
	PCareFile bm=(PCareFile)::ExAllocatePool(NonPagedPool,sizeof(CareFile));
	//PCareFile bm=(PCareFile)::ExAllocateFromNPagedLookasideList(&carefile_verify);
	//PWCHAR filenamebuffer=(PWCHAR)::ExAllocatePool(NonPagedPool,sizeof(WCHAR)*FILE_NAME_LENGTH_MAX);
	//::wcsncpy(filenamebuffer,filename.Buffer,FILE_NAME_LENGTH_MAX);

	RtlZeroMemory(bm,sizeof(CareFile));

	//RtlInitUnicodeString(&bm->filename,filenamebuffer);
	RtlInitUnicodeString(&bm->filepath,filepath->Buffer);
	BOOLEAN flag=FALSE;
	ULONG security_level=user_info.Security_Level;


	KdPrint(("DBAC_ControlLogic in!\n\n"));
	WCHAR result[124]={0};
	flag=CR_CheckIfCare(bm);
	do
	{
	if(flag)  //如果是关心文件
	{
		KdPrint(("Care File!\n"));
		status=0x1000;
		break;

		switch(irp_type)
		{
		case(IRP_MJ_SET_INFORMATION):
			if(security_level==ROLE_SYSTEM)//管理员
			{
				
				status= STATUS_SUCCESS;
				wcscpy(result,L"ROLE_SYSTEM :success\n\n");
				
			}
			else
			{
				if(user_info.UserID==bm->Owner)//文件属主
				{
				status= STATUS_SUCCESS;
				wcscpy(result,L"Owner :success\n\n");
				
				}
				if(bm->secutiry_level>=3)
				{
				status= STATUS_ACCESS_DENIED;
				wcscpy(result,L"security_level two high :denied!\n\n");
				
				}
				else
				{
				status= STATUS_SUCCESS;
				wcscpy(result,L"security_level <=2 :success\n\n");
				
				}
			}
			break;

		default:
				status= STATUS_SUCCESS;
				wcscpy(result,L"default :success\n\n");
				
			break;
		}
	}
	else     //非关心文件的安全等级为2
	{
		if(security_level==ROLE_GUEST) 
		{
				status= STATUS_ACCESS_DENIED;
				wcscpy(result,L"NOT CARE ROLE_GUEST :denied\n\n");
				break;
		}
		else
		{
				status= STATUS_SUCCESS;
				wcscpy(result,L"NOT CARE :success\n\n");
				break;
		}

	}
	}while(0);

	KdPrint(("result: %ws\n",result));
//	::ExFreePool((PVOID)filenamebuffer);
	bm->filepath.Buffer=0;
	::ExFreeToNPagedLookasideList(&carefile_verify,bm);
	return status;
}
NTSTATUS  DBAC_ControlLogicEx(PUNICODE_STRING &directory,ULONG irp_type,PFILE_OBJECT fileobject)
{
	NTSTATUS statu;
	PUNICODE_STRING  directory_buffer=(PUNICODE_STRING)::ExAllocatePool(PagedPool,sizeof(UNICODE_STRING));;
	PWCHAR directory_buffers=(PWCHAR)::ExAllocatePool(PagedPool,sizeof(WCHAR)*FILE_PATH_LENGTH_MAX);
	UNICODE_STRING dosname; //盘符
	directory=&fileobject->FileName; //得到路径

	do{
	if(::KeAreApcsDisabled())
	{
		KdPrint(("KeAreApcsDisabled failed!\n"));
		if(fileobject->DeviceObject==NULL)
		KdPrint(("DeviceObject is null!\n"));
		else
		{
		  statu=::RtlVolumeDeviceToDosName(fileobject->DeviceObject,&dosname);
		  if(NT_SUCCESS(statu))
		  {
		     KdPrint(("获得盘符:%ws!\n",dosname.Buffer));
		  }
		  else
		  {
			   KdPrint(("获得盘符失败!\n"));
			   break;
		  }
			
		}
	}
	else
	{
		if(fileobject->DeviceObject==NULL)
			KdPrint(("DeviceObject is null!\n"));
		else
		{
		  statu=::RtlVolumeDeviceToDosName(fileobject->DeviceObject,&dosname);
		  if(NT_SUCCESS(statu))
		  {
		     KdPrint(("获得盘符:%ws!\n",dosname.Buffer));

			 wcsncpy(directory_buffers,dosname.Buffer,dosname.Length);
			 ::RtlInitUnicodeString(directory_buffer,directory_buffers);   //初始化为一个带有盘符的字符串
			 directory_buffer->MaximumLength=FILE_PATH_LENGTH_MAX*sizeof(WCHAR);
			 statu=RtlAppendUnicodeStringToString(directory_buffer,directory);   //添加成完整路径
			 if(NT_SUCCESS(statu))
			 {
			   KdPrint(("获得完整路径:%ws!\n",directory_buffer->Buffer));

			 }
			 else
			 {
			    KdPrint(("获得完整路径出现错误:缓冲区太小!\n",directory_buffer->Buffer));
			 }		 
			 break;
		  }
		  else
		  {
			  KdPrint(("获得盘符失败:%0x!\n",statu));
			  break;
		  }
			

		}
	}
	 wcsncpy(directory_buffers,dosname.Buffer,dosname.Length);
	
	}while(0);
	
	 RtlInitUnicodeString(directory_buffer,directory_buffers);
	
   /* UNICODE_STRING name; //定义结构变量
	WCHAR nameBuffer[MAX_DEVNAME_LENGTH]={0}; //定义宽字符缓冲区,长度64
     RtlInitEmptyUnicodeString( &name, nameBuffer, sizeof(WCHAR)*MAX_DEVNAME_LENGTH ); //初始化name(成员Buffer->nameBuffer,Length=0,MaximumLength=64)

		PVPB vpb=fileobject->Vpb;
		if(vpb==NULL)
			KdPrint(("VPB IS NULL\n"));
		else{
		KdPrint(("打印参数快(VPB):--------------\n"));
		SfGetObjectName( vpb->DeviceObject, &name );
		KdPrint(("DeviceObject:%p name:%ws\n---",vpb->DeviceObject,name.Buffer));
		KdPrint(("name:%ws\n---",name.Buffer));
		SfGetObjectName( vpb->RealDevice, &name );
		KdPrint(("RealDevice:%p name:%ws\n---",vpb->RealDevice,name.Buffer));
		KdPrint(("name:%ws\n---",name.Buffer));
		KdPrint(("label:%ws \ntype:%d\n",vpb->VolumeLabel,vpb->Type));
		}
	*/

	//PUNICODE_STRING filename=(PUNICODE_STRING)::ExAllocatePool(NonPagedPool,sizeof(UNICODE_STRING));;
	// filenames=(PWCHAR)::ExAllocatePool(NonPagedPool,sizeof(WCHAR)*FILE_NAME_LENGTH_MAX);
//	::RtlInitEmptyUnicodeString(filename,filenames,FILE_NAME_LENGTH_MAX*sizeof(WCHAR));
	::PlayCurrentThreadName();

	//::GetFileNameFromDirectory(directory,filename);

	statu=DBAC_ControlLogic(directory_buffer,irp_type);

	::ExFreePool(directory_buffer);
	::ExFreePool(directory_buffers);
	

	return statu;
}
