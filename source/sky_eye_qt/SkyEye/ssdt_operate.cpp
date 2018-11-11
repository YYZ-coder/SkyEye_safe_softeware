#include "ssdt_operate.h"
#include <QMessageBox>
#include <QDebug>

QString ssdt_symbol = "\\\\.\\SSDTProcess";

SSDT_Operate::SSDT_Operate()
{
    DriverSSDT = NULL;
}

SSDT_Operate::~SSDT_Operate()
{

}

//打开驱动
bool SSDT_Operate::openDriver()
{
    DriverSSDT = CreateFileW( ssdt_symbol.toStdWString().data(),
                            GENERIC_READ | GENERIC_WRITE,
                            0, 0, OPEN_EXISTING,
                            FILE_ATTRIBUTE_SYSTEM, 0);
    if(DriverSSDT == INVALID_HANDLE_VALUE){
        return FALSE;
    }
    return TRUE;
}

//关闭驱动
void SSDT_Operate::closeSSDT()
{
    CloseHandle(DriverSSDT);
}

//清除驱动黑名单
bool SSDT_Operate::clear()
{
    bool send = false;
    int ret_len = 0;
    if (DeviceIoControl(DriverSSDT, IOCTL_SSDT_CLAER_BLACK_ILST, NULL, 0, NULL, 0, (DWORD*)&ret_len, 0))
        send = true;
    CloseHandle(DriverSSDT);
    return send;
}

//发送SSDT当前进程ID进行隐藏和保护
bool SSDT_Operate::addHideAndProtected(ULONG pid)
{
    bool send = false;
    int ret_len = 0;
    //获得当前进程的PID
    ULONG PID = pid;
    if (DeviceIoControl(DriverSSDT, IOCTL_SSDT_ADD, &PID, sizeof(ULONG), NULL, 0, (DWORD*)&ret_len, 0))
        send = true;
    CloseHandle(DriverSSDT);
    return send;
}

//删除当前进程ID取消隐藏和保护
bool SSDT_Operate::deleteHideAndProtected()
{
    int ret_len = 0;
    bool send = false;
    ULONG PID = ::getpid();
    if (DeviceIoControl(DriverSSDT, IOCTL_SSDT_DELETE, &PID, sizeof(ULONG), NULL, 0, (DWORD*)&ret_len, 0))
        send = true;
    CloseHandle(DriverSSDT);
    return send;
}

//增加禁止启动的进程名
bool SSDT_Operate::addForbidProcessName(QString pn)
{
    bool send = false;
    int ret_len = 0;

    ForbidProcessName FP;
    ZeroMemory(&FP,sizeof(FP));
    FP.length = pn.length();
    pn.toWCharArray(FP.ProcessName);

    if(openDriver()){
        if(DeviceIoControl(DriverSSDT,IOCTL_SSDT_FORBIDPROCESS_ADD,&FP,sizeof(ForbidProcessName),NULL,0,(DWORD*)&ret_len,0))
            send = true;
    }
    CloseHandle(DriverSSDT);
    return send;
}

//删除禁止启动的进程名
bool SSDT_Operate::deleteForbidProcessName(QString pn)
{
    bool send = false;
    int ret_len = 0;

    ForbidProcessName FP;
    ZeroMemory(&FP,sizeof(FP));
    FP.length = pn.length();
    pn.toWCharArray(FP.ProcessName);

    if(openDriver()){
        if(DeviceIoControl(DriverSSDT,IOCTL_SSDT_FORBIDPROCESS_DELETE,&FP,sizeof(ForbidProcessName),NULL,0,(DWORD*)&ret_len,0))
             send = true;
    }
    CloseHandle(DriverSSDT);
    return send;
}

//安装界面，用来设置注册表信息(dbname,dbhost,sysadmin,syspassword)
bool SSDT_Operate::sendRegistryData(QString sysAdmin, QString sysPassword, QString dbName, QString dbHost)
{
    bool send = false;
    int ret_len = 0;
    R3_TO_REG r2r;
    ZeroMemory(&r2r,sizeof(R3_TO_REG));
    //QString => WCHAR
    sysAdmin.toWCharArray(r2r.sysAdmin);
    sysPassword.toWCharArray(r2r.sysPassword);
    dbName.toWCharArray(r2r.dbName);
    dbHost.toWCharArray(r2r.dbHost);

    if (DeviceIoControl(DriverSSDT, IOCTL_SSDT_SET_REG, &r2r, sizeof(R3_TO_REG), NULL, 0, (DWORD*)&ret_len, 0))
        send = true;

    CloseHandle(DriverSSDT);
    return send;
}

//返回用来登陆数据库的数据
ForDB SSDT_Operate::getData()
{
    int ret_len = 0;

    ForDB s2s;
    s2s.username = "";
    s2s.password = "";
    s2s.host = "";
    s2s.dbname = "";

    CP_TO_R3 up;
    ZeroMemory(&up,sizeof(CP_TO_R3));
    if (DeviceIoControl(DriverSSDT, IOCTL_SSDT_GET_UP, NULL, 0, &up, sizeof(CP_TO_R3), (DWORD*)&ret_len, 0)){
        CloseHandle(DriverSSDT);
        QString user;
        QString password;
        QString host;
        QString dbname;

        qDebug()<<up.userLen<<"--"<<up.passLen;
        //username
        for(int i=0;i<up.userLen;i++){
            user += QString(QChar(up.USER[i]));
            qDebug()<<user<<endl;
        }

        //password
        for(int j=0;j<up.passLen;j++){
            password += QString(QChar(up.PASSWORD[j]));
            qDebug()<<password<<endl;
        }
        //host
        for(int j=0;j<10;j++){
            if(up.HOST[j] <= 'z' && up.HOST[j] >= '0'){
                host += QString(QChar(up.HOST[j]));
                qDebug()<<host<<endl;
            }else break;
        }
        //dbname
        for(int j=0;j<10;j++){
            if(up.DBNAME[j] <= 'z' && up.DBNAME[j] >= '0'){
                dbname += QString(QChar(up.DBNAME[j]));
                qDebug()<<dbname<<endl;
            }else break;
        }
        s2s.username = user;
        s2s.password = password;
        s2s.host = host;
        s2s.dbname = dbname;
    }
    CloseHandle(DriverSSDT);
    return s2s;
}

