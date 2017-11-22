// MMFIO.cpp : Defines the entry point for the console application.
//

#include "Win32MMFIO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MMF_ERR_ZERO_BYTE_FILE           "Cannot open zero byte file."
#define MMF_ERR_INVALID_SET_FILE_POINTER "The file pointer cannot be set to specified location."
#define MMF_ERR_WRONG_OPEN               "Close previous file before opening another."
#define MMF_ERR_OPEN_FILE                "Error encountered during file open."
#define MMF_ERR_CREATEFILEMAPPING        "Failed to create file mapping object."
#define MMF_ERR_MAPVIEWOFFILE            "Failed to map view of file."
#define MMF_ERR_SETENDOFFILE             "Failed to set end of file."
#define MMF_ERR_INVALIDSEEK              "Seek request lies outside file boundary."
#define MMF_ERR_WRONGSEEK                "Offset must be negative while seeking from file end."

//~~~ CWinMMFIO implementation ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Win32MMFIO::Win32MMFIO()
{
	SYSTEM_INFO sinf;
	GetSystemInfo(&sinf);
	m_dwAllocGranularity = sinf.dwAllocationGranularity;
	m_lExtendOnWriteLength = m_dwAllocGranularity;

	m_dwBytesInView = m_dwAllocGranularity;
	m_nCurPos = 0;
	m_nViewBegin = 0;
	m_ptr = 0;
	m_hmap = INVALID_HANDLE_VALUE;
	m_ref = 0;
	m_bFileExtended = false;
}

void Win32MMFIO::_close()
{
	_checkFileExtended();

	if(m_ptr)
	{
		//unmap view 
		FlushViewOfFile(m_ptr, 0);
		UnmapViewOfFile(m_ptr);
		m_ptr = NULL;
	}

	if(m_hmap)
	{
		//close mapping object handle
		CloseHandle(m_hmap);
		m_hmap = NULL;
	}

	if(m_hfile)
	{
		CloseHandle(m_hfile);
		m_hfile = NULL;
	}
}

Win32MMFIO::~Win32MMFIO()
{
	_close();
}

bool Win32MMFIO::Open(const sstring& strfile, OPENFLAGS oflags, suint64 mem_size)
{
#ifdef _DEBUG
	SysAssert(m_ref == 0);
#endif

	if(m_ref)
	{
		m_strErrMsg = MMF_ERR_WRONG_OPEN;
		return false;
	}
	m_ref++;


	DWORD flag_access = (OPN_READWRITE == oflags) ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
	m_hfile = CreateFile(strfile.c_str(), flag_access, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(INVALID_HANDLE_VALUE == m_hfile)
	{
		m_strErrMsg = MMF_ERR_OPEN_FILE;
		return false;
	}

	DWORD file_high;
	m_qwFileSize = GetFileSize(m_hfile, &file_high);
	m_qwFileSize += (((sint64) file_high) << 32);

	if(m_qwFileSize == 0)
	{
		suint64 nLength = mem_size;
		LONG    nLengthHigh = (nLength >> 32);
		DWORD   dwPtrLow = SetFilePointer(m_hfile, (LONG)(nLength & 0xFFFFFFFF), &nLengthHigh, FILE_BEGIN);
		SetEndOfFile(m_hfile);

		m_qwFileSize = GetFileSize(m_hfile, &file_high);
		m_qwFileSize += (((sint64)file_high) << 32);

		if(m_qwFileSize == 0)
		{
			CloseHandle(m_hfile);
			m_strErrMsg = MMF_ERR_ZERO_BYTE_FILE;
			return false;
		}
	}

	// Create the file-mapping object.
	m_flag_map = (OPN_READWRITE == oflags) ? PAGE_READWRITE : PAGE_READONLY;
	m_hmap = CreateFileMapping(m_hfile, NULL, m_flag_map, 0, 0, 0);

	if(NULL == m_hmap)
	{
		if(INVALID_HANDLE_VALUE == m_hfile)
			CloseHandle(m_hfile);
		m_strErrMsg = MMF_ERR_CREATEFILEMAPPING;
		return false;
	}

	if(m_qwFileSize <= m_dwBytesInView)
		m_dwBytesInView = m_qwFileSize;


	m_dwflagsView = (OPN_READ == oflags) ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS;
	m_ptr = (PBYTE)MapViewOfFile(m_hmap, m_dwflagsView, 0, 0, m_dwBytesInView);

	if(NULL == m_ptr)
	{
		CloseHandle(m_hmap);
		m_strErrMsg = MMF_ERR_MAPVIEWOFFILE;
		return false;
	}

	m_nCurPos = 0;
	m_nViewBegin = 0;
	m_bFileExtended = false;

	return true;
}

bool Win32MMFIO::Close()
{
#ifdef _DEBUG
	SysAssert(m_ref == 1);
#endif

	m_ref--;
	_close();
	return true;
}


int Win32MMFIO::Read(void* pBuf, int nCountIn)
{
	if(nCountIn ==0)return 0;

	_checkFileExtended();

	if(m_nCurPos >= m_qwFileSize)return 0;

	int nCount = nCountIn;//int is used to detect any bug

	m_dwBytesInView = m_dwAllocGranularity;
	//check if m_nViewBegin+m_dwBytesInView crosses filesize
	if(m_nViewBegin + m_dwBytesInView > m_qwFileSize)
	{
		m_dwBytesInView = m_qwFileSize - m_nViewBegin;
	}

	sint64 nDataEndPos = m_nCurPos + nCount;
	if(nDataEndPos >= m_qwFileSize)
	{
		nDataEndPos = m_qwFileSize;
		nCount = m_qwFileSize - m_nCurPos;
	}

	SysAssert(nCount >= 0);//nCount is int, if -ve then error

	sint64 nViewEndPos = m_nViewBegin + m_dwBytesInView;

	if(nDataEndPos < nViewEndPos)
	{
		memcpy_s(pBuf, nCountIn, m_ptr+ (m_nCurPos-m_nViewBegin), nCount);
		m_nCurPos += nCount;
	}
	else if(nDataEndPos == nViewEndPos)
	{
		//Last exact bytes are read from the view
		memcpy_s(pBuf, nCountIn, m_ptr+ (m_nCurPos-m_nViewBegin), nCount);
		m_nCurPos += nCount;

		_seek(m_nCurPos, SP_BEGIN);
		nViewEndPos = m_nViewBegin + m_dwBytesInView;
	}
	else
	{
		LPBYTE pBufRead = (LPBYTE)pBuf;
		if(nDataEndPos > nViewEndPos)
		{
			//nDataEndPos can span multiple view blocks
			while(m_nCurPos < nDataEndPos)
			{
				int nReadBytes = nViewEndPos - m_nCurPos;

				if(nViewEndPos > nDataEndPos)
					nReadBytes = nDataEndPos - m_nCurPos;

				memcpy_s(pBufRead, nCountIn, m_ptr + (m_nCurPos-m_nViewBegin), nReadBytes);
				pBufRead += nReadBytes;

				m_nCurPos += nReadBytes;
				//seeking does view remapping if m_nCurPos crosses view boundary
				_seek(m_nCurPos, SP_BEGIN);
				nViewEndPos = m_nViewBegin + m_dwBytesInView;
			}
		}
	}

	return nCount;
}

bool Win32MMFIO::SetLength(const sint64& nLength)
{
	//unmap view 
	UnmapViewOfFile(m_ptr);
	//close mapping object handle
	CloseHandle(m_hmap);

	LONG nLengthHigh = (nLength >> 32);
	DWORD dwPtrLow = SetFilePointer(m_hfile, (LONG) (nLength & 0xFFFFFFFF), &nLengthHigh, FILE_BEGIN);

	if(INVALID_SET_FILE_POINTER == dwPtrLow && GetLastError() != NO_ERROR)
	{
		m_strErrMsg = MMF_ERR_INVALID_SET_FILE_POINTER;
		return false;
	}
	//set the eof to the file pointer position
	if(SetEndOfFile(m_hfile) == 0)
	{
		m_strErrMsg = MMF_ERR_SETENDOFFILE;
		return false;
	}

	m_qwFileSize = nLength;

	//call CreateFileMapping 
	m_hmap = CreateFileMapping(m_hfile, NULL, m_flag_map, 0, 0, "SMP");

	//remap here
	m_ptr = (PBYTE)MapViewOfFile(m_hmap, m_dwflagsView, (DWORD) (m_nViewBegin >> 32), (DWORD) (m_nViewBegin & 0xFFFFFFFF), m_dwBytesInView);
	return true;
}

int Win32MMFIO::Write(void* pBuf, int nCount)
{
	if(nCount == 0)return 0;

	sint64 nViewEndPos = m_nViewBegin + m_dwBytesInView;
	sint64 nDataEndPos = m_nCurPos + nCount;

	if(nDataEndPos > nViewEndPos)
	{
		if(nDataEndPos >= m_qwFileSize)
		{
			//Extend the end position by m_lExtendOnWriteLength bytes
			sint64 nNewFileSize = nDataEndPos + m_lExtendOnWriteLength;

			if(SetLength(nNewFileSize))
			{
				m_bFileExtended = true;
			}
		}

		LPBYTE pBufWrite = (LPBYTE)pBuf;
		while(m_nCurPos < nDataEndPos)
		{
			int nWriteBytes = nViewEndPos - m_nCurPos;

			if(nViewEndPos > nDataEndPos)
				nWriteBytes = nDataEndPos - m_nCurPos;

			memcpy_s(m_ptr +(m_nCurPos-m_nViewBegin), m_dwBytesInView, pBufWrite, nWriteBytes);
			pBufWrite += nWriteBytes;

			m_nCurPos += nWriteBytes;
			//seeking does view remapping if m_nCurPos crosses view boundary
			_seek(m_nCurPos, SP_BEGIN);
			nViewEndPos = m_nViewBegin + m_dwBytesInView;
		}
	}
	else
	{
		//nCount bytes lie within the current view
		memcpy_s(m_ptr + (m_nCurPos-m_nViewBegin), nCount, pBuf, nCount);
		m_nCurPos += nCount;
	}

	return nCount;
}

void Win32MMFIO::_flush()
{
}

bool Win32MMFIO::Seek(sint64 lOffset/*can be -ve */, SEEKPOS eseekpos)
{
	_checkFileExtended();
	bool bRet = _seek(lOffset, eseekpos);
	return bRet;
}

bool Win32MMFIO::_seek(sint64 lOffset/*can be -ve */, SEEKPOS eseekpos)
{
	if(SP_CUR == eseekpos)
	{
		lOffset = m_nCurPos + lOffset;
	}
	else if(SP_END == eseekpos)
	{
		if(lOffset >= 0)
		{
			m_strErrMsg = MMF_ERR_WRONGSEEK;
			return false;
		}

		//lOffset in RHS is -ve
		lOffset = m_qwFileSize + lOffset;
	}
	//else means SP_BEGIN


	//lOffset must be less than the file size
	if(!(lOffset >= 0 && lOffset < m_qwFileSize))
	{
		m_strErrMsg = MMF_ERR_INVALIDSEEK;
		return false;
	}

	if(!(lOffset >= m_nViewBegin && lOffset < m_nViewBegin + m_dwBytesInView))
	{
		//lOffset lies outside the mapped view, remap the view
		sint64 _N = (sint64)floor((double)lOffset/((double)m_dwAllocGranularity));
		m_nViewBegin = _N*m_dwAllocGranularity;
		m_dwBytesInView = m_dwAllocGranularity;
		//check if m_nViewBegin+m_dwBytesInView crosses filesize
		if(m_nViewBegin + m_dwBytesInView > m_qwFileSize)
		{
			m_dwBytesInView = m_qwFileSize - m_nViewBegin;
		}
		if(m_dwBytesInView != 0 && m_ptr)
		{
			//Unmap old view
			UnmapViewOfFile(m_ptr);
			//Remap with new starting address
			m_ptr = (PBYTE)MapViewOfFile(m_hmap, m_dwflagsView, (DWORD) (m_nViewBegin >> 32), (DWORD) (m_nViewBegin & 0xFFFFFFFF), m_dwBytesInView);
		}
	}

	m_nCurPos = lOffset;
	return true;
}

suint64 Win32MMFIO::GetLength()
{
	_checkFileExtended();
	return m_qwFileSize;
}

suint64 Win32MMFIO::GetPosition()
{
	return m_nCurPos;
}

/*
If file is extended in Write function then this must be called to re-adjust
the file to its actual length before Seek or Read or any such fuction.
*/
bool Win32MMFIO::_checkFileExtended()
{
	bool bRet = true;
	if(m_bFileExtended)
	{
		//remove extra bytes
		bRet = SetLength(m_nCurPos);
	}
	m_bFileExtended = false;
	return bRet;
}
