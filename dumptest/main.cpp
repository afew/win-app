
#include <stdio.h>

#include "LpwMiniDump.h"


class CTestCls
{
public:
	int isOpened;
};



void TestFunc()
{
	CTestCls* testCls = NULL;

	testCls->isOpened = 0;
}


static void* g_prevExceptionFilter = LpwMiniDumpCreate();


int main()
{
	TestFunc();	

	LpwMiniDumpRelease(g_prevExceptionFilter);
	return 0;
}

