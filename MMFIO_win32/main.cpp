// MMFIO.cpp : Defines the entry point for the console application.
//

#include "Win32MMFIO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int main(int argc, char* argv[], char* envp[])
{
	int nRetCode = 0;
	Win32MMFIO mmf_read;
	Win32MMFIO mmf_write;
	bool bRet = mmf_read.Open("C:/read.json", Win32MMFIO::OPN_READ);
	if(!bRet)return false;

	suint64 f_len = mmf_read.GetLength();
	bRet = mmf_write.Open("C:/writefile.txt", Win32MMFIO::OPN_READWRITE, f_len);
	if(!bRet)return false;

	char tmp_buf[4096];


	int ncount=0, nwrite=0;
	while(1)
	{
		ncount = mmf_read.Read(tmp_buf, 4096);
		if(!ncount)
			break;

		nwrite = mmf_write.Write(tmp_buf, ncount);
	}

	suint64 nWriteFileLength = mmf_write.GetLength();

	mmf_read.Close();
	mmf_write.Close();

	return nRetCode;
}
