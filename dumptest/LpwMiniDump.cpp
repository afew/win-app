
#pragma warning(disable: 4996)

//#include <crtdbg.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <DbgHelp.h>

#include "LpwMiniDump.h"


typedef int (WINAPI *FPTR_DUMPWRITER)
(
	HANDLE, DWORD, HANDLE, MINIDUMP_TYPE
	, CONST PMINIDUMP_EXCEPTION_INFORMATION
	, CONST PMINIDUMP_USER_STREAM_INFORMATION
	, CONST PMINIDUMP_CALLBACK_INFORMATION
);


static LONG WINAPI CbExceptionFilter(PEXCEPTION_POINTERS exInfo)
{
	HMODULE							hDll = LoadLibrary("DBGHELP.DLL");
	SYSTEMTIME						sysTime;
	int								d_year, d_month, d_day, d_hour, d_minute, d_second;

	char							path[MAX_PATH]={0};
	HANDLE							hFile = NULL;
	MINIDUMP_EXCEPTION_INFORMATION	dumpInfo;

	int								hr = 0;


	if(NULL == hDll)
	{
		printf("ERR::MINIDUMP::LoadLibrary::DBGHELP.DLL\n");
		return EXCEPTION_CONTINUE_SEARCH;
	}

	FPTR_DUMPWRITER FuncDump = (FPTR_DUMPWRITER)GetProcAddress(hDll, "MiniDumpWriteDump");
	if(NULL == FuncDump)
	{
		printf("ERR::MINIDUMP::GetProcAddress::FPTR_DUMPWRITER\n");
		return EXCEPTION_CONTINUE_SEARCH;
	}

	GetLocalTime(&sysTime);

	d_year   = sysTime.wYear;
	d_month  = sysTime.wMonth;
	d_day    = sysTime.wDay;
	d_hour   = sysTime.wHour;
	d_minute = sysTime.wMinute;
	d_second = sysTime.wSecond;

	sprintf(path, "%d-%d-%d %d_%d_%d.dmp", d_year, d_month, d_day, d_hour, d_minute, d_second);


	hFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("ERR::MINIDUMP::CreateFile::%s\n", path);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	
	dumpInfo.ThreadId			= GetCurrentThreadId();
	dumpInfo.ExceptionPointers	= exInfo;
	dumpInfo.ClientPointers		= NULL;

	hr = FuncDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &dumpInfo, NULL, NULL);

	CloseHandle(hFile);


	if(hr)										// success
		return EXCEPTION_EXECUTE_HANDLER;

	return EXCEPTION_CONTINUE_SEARCH;			// other
}

void* LpwMiniDumpCreate()
{
	LPTOP_LEVEL_EXCEPTION_FILTER ret = NULL;

	//_CrtSetReportMode(_CRT_ASSERT, 0);
	//SetErrorMode(GetErrorMode () | SEM_NOGPFAULTERRORBOX);

	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOALIGNMENTFAULTEXCEPT|SEM_NOOPENFILEERRORBOX);
	ret = SetUnhandledExceptionFilter(CbExceptionFilter);

	return ret;
}


int LpwMiniDumpRelease(void* v)
{
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)v);
	return TRUE;
}

