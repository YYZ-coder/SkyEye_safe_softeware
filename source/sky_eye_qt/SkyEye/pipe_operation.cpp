#include "pipe_operation.h"

Pipe_Operation::Pipe_Operation()
{
    pipe = NULL;
}

Pipe_Operation::~Pipe_Operation()
{
    CloseHandle(pipe);
}

//创建pipe
bool Pipe_Operation::createPipe()
{
    bool fas = false;

    do{
        pipe = CreateNamedPipeA("\\\\.\\pipe\\test",
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_BYTE, 1, 1024, 1024,0 , NULL);
        //检查是否创建成功
        if (pipe == INVALID_HANDLE_VALUE){
           break;
        }
        fas = true;
        //异步IO结构
        OVERLAPPED op;
        ZeroMemory(&op, sizeof(OVERLAPPED));
        //创建一个事件内核对象
        op.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        //等待一个客户端进行连接
        if(!ConnectNamedPipe(pipe, &op))
            break;
        //当有客户端进行连接时，事件变成有信号的状态
        if (WaitForSingleObject(op.hEvent, INFINITE) == 0){
            return true;
        }else break;
    }while(fas);

    return fas;
}

//关闭pipe
bool Pipe_Operation::closePipe()
{
    //通信完之后，断开连接
    DisconnectNamedPipe(pipe);
    //关闭管道
    return CloseHandle(pipe);
}

//写入非细节控制文件数据
bool Pipe_Operation::transferDate(PCareFile_T care)
{
    if(care==NULL) {
        return false;
    }
    int size=(sizeof(WCHAR))/(sizeof(char));
    WCHAR filepath[FILE_PATH_LENGTH_MAX];
    WCHAR filename[FILE_NAME_LENGTH_MAX];
    char *filepath_an=(char*)malloc(size*FILE_PATH_LENGTH_MAX);
    char *filename_an=(char*)malloc(size*FILE_NAME_LENGTH_MAX);
    int owner = 0;            //用户ID
    CHAR sendbuffer[4048];    //发送字符串
    int security_level = 0;   //安全等级
    ZeroMemory(sendbuffer,4048);

    wcsncpy(filepath,care->filepath,FILE_PATH_LENGTH_MAX);
    wcsncpy(filename,care->filename,FILE_NAME_LENGTH_MAX);
    Unicode2Ansi(filepath,filepath_an);
    Unicode2Ansi(filename,filename_an);

    owner=care->Owner;
    security_level=care->secutiry_level;

    sprintf(sendbuffer,"*filepath:%s|filename:%s|owner:%d|security_level:%d|#",
        filepath_an,
        filename_an,
        owner,
        security_level);

    DWORD writenum;
    if(WriteFile(pipe,sendbuffer,strnlen(sendbuffer,4048)-1,&writenum,NULL)){
        return true;
    }else{
        char buffer[1024];
        sprintf(buffer,"%d",GetLastError());
        QMessageBox::warning(NULL,"",QString::fromLatin1(buffer));
        return false;
    }
    return false;
}

//传输文件细节控制
bool Pipe_Operation::transferFileDetail(QString s,QString filep,int userid,int processID)
{
    //操作字符
    int fname = 0,fmove = 0,fdelete = 0,fread = 0,fwrite = 0,fall = 0;
    //字符串为空直接返回
    if(s.trimmed() == "" || s == "")
        return false;
    //依照‘,’分开字符串
    QStringList sl = s.split(",");
    //判断字符的存在与否
    if(sl.contains("FILE_RENAME")){fname = 1;}
    if(sl.contains("FILE_MOVE")){fmove = 1;}
    if(sl.contains("FILE_DELETE")){fdelete = 1;}
    if(sl.contains("FILE_READ")){fread = 1;}
    if(sl.contains("FILE_WRITE")){fwrite = 1;}
    if(sl.contains("FILE_ALL")){fall = 1;}

    //路径读取
    char *fpp;
    fpp = filep.toLatin1().data();
    //字符串不为空
    //进行包装,发送
    CHAR sendbuffer[4048];    //发送字符串
    ZeroMemory(sendbuffer,4048);
    sprintf(sendbuffer,"*FILE_PATH:%s|FILE_READ:%d|FILE_WRITE:%d|FILE_MODIFY:%d|FILE_ALL:%d|FILE_LOCK:%d|FILE_UNLOCK:%d|FILE_EXECUTE:%d|FILE_COPY:%d|USER_ID:%d|PROCESS_ID:%d|PROCESS_PERMIT:%d|FILE_RENAME:%d|FILE_MOVE:%d|FILE_DELETE:%d |?",
        fpp,
        fread,
        fwrite,
            0,
        fall,
            0,
            0,
            0,
            0,
        userid,
        processID,
            1,
        fname,
        fmove,
        fdelete);
    //发送字符串
    DWORD writenum;
    if(WriteFile(pipe,sendbuffer,strnlen(sendbuffer,4048)-1,&writenum,NULL)){
        return true;
    }else{
        return false;
    }
}

//传输目录控制
bool Pipe_Operation::transferDirDetail(QString s,QString dpath,int userid,int processID)
{
    //操作字符
    int dname = 0,dnew = 0,dmove = 0,ddelete = 0,dread = 0,dwrite = 0,dall = 0;
    //字符串为空直接返回
    if(s.trimmed() == "" || s == "")
        return false;
    //依照‘,’分开字符串
    QStringList sl = s.split(",");
    //判断字符的存在与否
    if(sl.contains("DIR_RENAME")){dname = 1;}
    if(sl.contains("DIR_NEW")){dnew = 1;}
    if(sl.contains("DIR_MOVE")){dmove = 1;}
    if(sl.contains("DIR_ALL")){dall = 1;}
    if(sl.contains("DIR_WRITE")){dwrite = 1;}
    if(sl.contains("DIR_READ")){dread = 1;}
    if(sl.contains("DIR_DELETE")){ddelete = 1;}

    //字符串不为空
    //进行包装,发送
    char *dpp;
    dpp = dpath.toLatin1().data();

    CHAR sendbuffer[4048];    //发送字符串
    ZeroMemory(sendbuffer,4048);
    sprintf(sendbuffer,"*DIRECTORY_ID:%s|DIRECTORY_NAMES:%s|DIRECTORY_MODIFY_RENAME:%d|DIRECTORY_MODIFY_MOVE:%d|DIRECTORY_MODIFY_DELETE:%d|DIRECTORY_MODIFY_NEWFILE:%d|DIRECTORY_ACCESS:%d|DIRECTORY_READ:%d|OBJECT_ID:%d|PROCESS_IDENTIFY:%d|PROCESS_PERMIT:%d|?",
        dpp,
        dpp,
        dname,
        dmove,
        ddelete,
        dnew,
        dwrite,
        dread,
        userid,
        processID,
         0);
    //发送字符串
    DWORD writenum;
    if(WriteFile(pipe,sendbuffer,strnlen(sendbuffer,4048)-1,&writenum,NULL)){
        return true;
    }else
        return false;
}

//获得后台输出
bool Pipe_Operation::getNext()
{
    bool yes = false;
    char message[4048];
    DWORD size;
    for(int i=0;i<2000;i++){
        if(ReadFile(pipe,message,4048,&size,NULL)){
            if(strcmp(message,"ACCEPT") == 0) return true;
        }
    }
    return false;
}

//发送关闭消息
bool Pipe_Operation::closeSubProcess()
{
    char exit[4];
    DWORD n =0;
    strcpy(exit,"EXIT");
    if(WriteFile(pipe,exit,4,&n,NULL)){
        return true;
    }else
        return false;
    return false;
}

//字符转换
bool Pipe_Operation::Unicode2Ansi(PWCHAR pUnicode, PCHAR pAnsi)
{
    DWORD dwNum = 0;
    BOOL bRet = FALSE;
    do{
        // 检查参数
        if (pAnsi == NULL || pUnicode == NULL || wcslen(pUnicode) == 0)
            break;

        dwNum = WideCharToMultiByte(CP_ACP,0,pUnicode,-1,NULL,0,0,0);

        if (WideCharToMultiByte(CP_ACP,0, pUnicode, -1, pAnsi, dwNum,0,0)){
            bRet = TRUE;
        }else{
            break;
        }
    }while (FALSE);

    return bRet;
}

