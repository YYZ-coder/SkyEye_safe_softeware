#include "DriverOperation.h"
#include "DataBaseOperation.h"
#include "resource.h"
#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include "winsock.h"
#include <mysql.h>



BOOL DataBaseOperation::OpenDB(CHAR *chu, CHAR *chp,CHAR *dbhost,CHAR *dbname){
	BOOL Opened = FALSE;
	MYSQL * con;
	con = mysql_init((MYSQL*)0);
	if (con != NULL &&
		mysql_real_connect(con, dbhost, chu, chp, dbname, 3306/*TCP IP端口*/, NULL/*Unix Socket 连接类型*/, 0/*运行成ODBC数据库标志*/))
	{
		Opened = TRUE;
	}
	mysql_close(con);
	return Opened;
}

//开始新建驱动对象，并对驱动进行传规则和用户名密码操作
BOOL DataBaseOperation::OperateDriver(){
	return FALSE;
}