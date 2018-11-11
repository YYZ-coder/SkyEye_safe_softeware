
#include <windows.h>
#include "common.h"

class DataBaseOperation{
public:
	DataBaseOperation(){}
	~DataBaseOperation(){}
	BOOL OpenDB(CHAR *wchu, CHAR *wchp, CHAR *dbhost, CHAR *dbname);
	BOOL OperateDriver();


};