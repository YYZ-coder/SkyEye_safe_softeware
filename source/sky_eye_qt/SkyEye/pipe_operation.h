#ifndef PIPE_OPERATION_H
#define PIPE_OPERATION_H

#include <windows.h>
#include <stdio.h>
#include <QMessageBox>
#define FILE_NAME_LENGTH_MAX 512
#define FILE_PATH_LENGTH_MAX 1024
//传递的结构体
typedef struct _CareFile_T
{
    WCHAR filename[FILE_NAME_LENGTH_MAX];
    WCHAR filepath[FILE_PATH_LENGTH_MAX];

    INT            secutiry_level;
    ULONG          Owner;
    ULONG          access_times;
    ULONG          refused_times;
}CareFile_T,*PCareFile_T;

//对进程管道进行操作
class Pipe_Operation
{
public:
    Pipe_Operation();
    ~Pipe_Operation();

    bool createPipe();

    bool createAnonymousPipe();

    bool closePipe();

    bool closeSubProcess();

    bool transferDate(PCareFile_T care);

    bool transferFileDetail(QString s,QString fp,int userid,int processID);

    bool transferDirDetail(QString s,QString dpath,int userid,int processID);

    bool getNext();

private:

    HANDLE pipe;

    bool Unicode2Ansi( PWCHAR pUnicode, PCHAR pAnsi);

};

#endif // PIPE_OPERATION_H
