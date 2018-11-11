#include <QCoreApplication>
#include <QApplication>
#include <windows.h>
#include <QDebug>

#define WFP_DRIVER_NAME "WFP_Filter_Driver"
QString WFP_DRIVER_SYMBOL_NAME = "\\\\.\\wfp_device";


#define SSDT_DRIVER_NAME "SSDT_Hook"
QString SSDT_DRIVER_SYMBOL_NAME = "\\\\.\\SSDTProcess";

#define SFILTER_DRIVER_NAME "SFilter"
QString SFILTER_DRIVER_SYMBOL_NAME = "\\\\.\\sfilter_device";

//加载驱动
BOOL loadDriver(char * lpszDriverName, char * lpszDriverPath){

    char szDriverImagePath[256];
    printf("DriverName:%s\n",lpszDriverName);

    //得到完整路径并拷贝至szDriverImagePath
    GetFullPathNameA(lpszDriverPath, 256, szDriverImagePath, NULL);

    printf("path:%s\n",lpszDriverPath);

    BOOL bRet = FALSE;

    SC_HANDLE hServiceMgr = NULL;
    SC_HANDLE hServiceDDK = NULL;

    //打开服务控制管理器
    hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hServiceMgr == NULL){
        //openSCManager失败
        printf("openSCManager() Failed! %d \n", GetLastError());
        bRet = FALSE;
        goto BeforeLeave;
    }
    else{
        printf("OpenSCManager() OK!\n");
    }

    //创建驱动对应服务
    hServiceDDK = CreateServiceA(hServiceMgr,
        lpszDriverName,
        lpszDriverName,
        SERVICE_ALL_ACCESS,
        SERVICE_FILE_SYSTEM_DRIVER,
        SERVICE_SYSTEM_START,//开机便启动
        SERVICE_ERROR_IGNORE,
        szDriverImagePath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    DWORD dwRtn;
    //判断服务失败
    if (hServiceDDK == NULL){
        dwRtn = GetLastError();
        if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS){
            printf("CreateService() Failed %d!\n", dwRtn);
            bRet = FALSE;
            goto BeforeLeave;
        }
        else{
            printf("CreateService() Failed Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS!\n");
        }

        hServiceDDK = OpenServiceA(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);

        if (hServiceDDK == NULL){
            dwRtn = GetLastError();
            printf("OpenService() Failed %d! \n", dwRtn);
            bRet = FALSE;
            goto BeforeLeave;
        }
        else{
            printf("OpenSerice() ok!\n");
        }
    }
    else{
        printf("CreateService() ok !\n");
    }

    //开启此项服务
    bRet = StartService(hServiceDDK, NULL, NULL);
    if (!bRet){
        DWORD dwRtn = GetLastError();
        if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING){
            printf("Start Service Failed! %d\n",GetLastError());
            bRet = FALSE;
            goto BeforeLeave;
        }
        else{
            if (dwRtn == ERROR_IO_PENDING){
                printf("StartService() Failed ERROR_IO_PENDING!\n");
                bRet = FALSE;
                goto BeforeLeave;
            }
            else{
                printf("StartService FAILED ! ERROR_SERVICE_ALREADY_RUNNING \n");
                bRet = TRUE;
                goto BeforeLeave;
            }
        }
    }

    //离开前关闭句柄
BeforeLeave:
    if (hServiceDDK){
        CloseServiceHandle(hServiceDDK);
    }
    if (hServiceMgr){
        CloseServiceHandle(hServiceMgr);
    }

    return bRet;

}

BOOL loadWFPDriver(){
    char *szDriverImagePath;
    char *lpszDriverName = "wfp_driver";
    printf("DriverName:%s\n",lpszDriverName);
    char *lpszDriverPath;

    //获得当前目录名
    QString path = QApplication::applicationDirPath();
    QByteArray ba = path.toLatin1();
    char *dn = ba.data();
    QString wfpdp = path + "/WFP_Filter.sys";
    lpszDriverPath = wfpdp.toLatin1().data();
    szDriverImagePath = wfpdp.toLatin1().data();

    //得到完整路径并拷贝至szDriverImagePath
    //GetFullPathNameA(lpszDriverPath, 256, szDriverImagePath, NULL);

    printf("path:%s\n",lpszDriverPath);

    BOOL bRet = FALSE;

    SC_HANDLE hServiceMgr = NULL;
    SC_HANDLE hServiceDDK = NULL;

    //打开服务控制管理器
    hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hServiceMgr == NULL){
        //openSCManager失败
        printf("WFP :openSCManager() Failed! %d \n", GetLastError());
        bRet = FALSE;
        goto BeforeLeave;
    }
    else{
        printf("WFP :OpenSCManager() OK!\n");
    }

    //创建驱动对应服务
    hServiceDDK = CreateServiceA(hServiceMgr,
        lpszDriverName,
        lpszDriverName,
        SERVICE_ALL_ACCESS,
        SERVICE_FILE_SYSTEM_DRIVER,
        SERVICE_DEMAND_START,//开机便启动
        SERVICE_ERROR_IGNORE,
        szDriverImagePath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    DWORD dwRtn;
    //判断服务失败
    if (hServiceDDK == NULL){
        dwRtn = GetLastError();
        if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS){
            printf("WFP :CreateService() Failed %d!\n", dwRtn);
            bRet = FALSE;
            goto BeforeLeave;
        }
        else{
            printf("WFP :CreateService() Failed Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS!\n");
        }

        hServiceDDK = OpenServiceA(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);

        if (hServiceDDK == NULL){
            dwRtn = GetLastError();
            printf("WFP :OpenService() Failed %d! \n", dwRtn);
            bRet = FALSE;
            goto BeforeLeave;
        }
        else{
            printf("WFP :OpenSerice() ok!\n");
        }
    }
    else{
        printf("WFP :CreateService() ok !\n");
    }

    //开启此项服务
    bRet = StartService(hServiceDDK, NULL, NULL);
    if (!bRet){
        DWORD dwRtn = GetLastError();
        if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING){
            printf("WFP :Start Service Failed! %d\n",GetLastError());
            bRet = FALSE;
            goto BeforeLeave;
        }
        else{
            if (dwRtn == ERROR_IO_PENDING){
                printf("WFP :StartService() Failed ERROR_IO_PENDING!\n");
                bRet = FALSE;
                goto BeforeLeave;
            }
            else{
                printf("WFP :StartService FAILED ! ERROR_SERVICE_ALREADY_RUNNING \n");
                bRet = TRUE;
                goto BeforeLeave;
            }
        }
    }

    //离开前关闭句柄
BeforeLeave:
    if (hServiceDDK){
        CloseServiceHandle(hServiceDDK);
    }
    if (hServiceMgr){
        CloseServiceHandle(hServiceMgr);
    }

    return bRet;
}

//测试驱动是否已经打开
BOOL testDriver(QString symbol_name){
    BOOL yes = false;
    HANDLE hDevice = CreateFileW(
                symbol_name.toStdWString().data(),
                GENERIC_WRITE | GENERIC_READ,0,
                NULL,OPEN_EXISTING,0,NULL);
        if (hDevice != INVALID_HANDLE_VALUE){
            yes = true;
        }
        else{
            printf("open Driver failed! %d \n",GetLastError());
        }
    CloseHandle(hDevice);
    return yes;
}

//主函数
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //获得当前目录名
    QString path = QApplication::applicationDirPath();
    QByteArray ba = path.toLatin1();
    char *dn = ba.data();
    printf("%s\n",dn);

    //char *wfpcn = NULL;
    char *ssdtcn = NULL;
    char *sfiltercn = NULL;

    //wfp.sys全路径名
    //QString wfpn = path + "/WFP_Filter.sys";
    //ssdt.sys全路径名
    QString ssdtn = path + "/Ssdt/SSDT_HOOK.sys";
    //sfilter.sys全路径名
    QString sfiltern = path + "/SFilter.sys";
    ///////////////////
    //qDebug()<<"WFP cn :"+wfpn<<endl;
    ///////////////////
    //转char*
    //wfpcn = wfpn.toLatin1().data();
    ssdtcn = ssdtn.toLatin1().data();
    sfiltercn = sfiltern.toLatin1().data();

    //printf("wfpcn : %s\n",wfpcn);
    printf("ssdtcn : %s\n",ssdtcn);
    printf("sfiltercn : %s\n",sfiltercn);

    //加载SSDT_HOOK
    if(loadDriver(SSDT_DRIVER_NAME,ssdtcn)){
        qDebug()<<"Load SSDT_hook Successful!"<<endl;
    }else{
        qDebug()<<"Load SSDT_hook Failed! Please Contact Admin!"<<endl;
    }
    //加载WFP_Filter
    if(loadWFPDriver()){
        qDebug()<<"Load WFP_Filter Successful!"<<endl;
    }else{
        qDebug()<<"Load WFP_Filter Failed! Please Contact Admin!"<<endl;
    }
    //加载SFilter
    if(loadDriver(SFILTER_DRIVER_NAME,sfiltercn)){
        qDebug()<<"Load SFilter Successful!"<<endl;
    }else{
        qDebug()<<"Load SFilter Failed! Please Contact Admin!"<<endl;
    }

    //测试驱动
    //wfp
    if(testDriver(WFP_DRIVER_SYMBOL_NAME)){
        qDebug()<<"WFP Driver Test Ok!"<<endl;
    }

    //ssdt
    if(testDriver(SSDT_DRIVER_SYMBOL_NAME)){
        qDebug()<<"SSDT Driver Test Ok!"<<endl;
    }

    //SFilter
    if(testDriver(SFILTER_DRIVER_SYMBOL_NAME)){
        qDebug()<<"SFilter Driver Test Ok!"<<endl;
    }

    system("PAUSE");

    return a.exec();
}
