#pragma once

#include <string>
#include <valarray>
#include <vector>
#include <assert.h>
#include <windows.h>

using namespace std;

typedef long long           sint64;
typedef unsigned long long suint64;
typedef std::string sstring;
#define SysAssert assert

class Win32MMFIO
{
public:
	enum SEEKPOS
	{
		SP_BEGIN = 0,
		SP_CUR,
		SP_END,
	};

	enum OPENFLAGS
	{
		OPN_READ = 0, // Opens the file for reading only
		OPN_READWRITE // Opens the file for reading and writing
	};
private:
	BYTE		m_ref;
	OPENFLAGS	m_eOpenflags;
	HANDLE		m_hmap;
	PBYTE		m_ptr;
	DWORD		m_dwBytesInView;
	sint64		m_qwFileSize;
	sint64		m_nViewBegin;//from begining of file
	sint64		m_nCurPos;//from begining of file
	HANDLE		m_hfile;
	DWORD		m_dwAllocGranularity;
	LONG		m_lExtendOnWriteLength;
	DWORD		m_flag_map;
	DWORD		m_dwflagsView;
	bool		m_bFileExtended;
	sstring		m_strErrMsg;

	void _flush();
	bool _checkFileExtended();
	bool _seek(sint64 lOffset, SEEKPOS eseekpos);
	void _close();

public:
	Win32MMFIO();
	virtual ~Win32MMFIO();

	/* Construction */
	bool Open(const sstring& strfile, OPENFLAGS oflags, suint64 mem_size=0);
	bool Close();

	/* I/O */
	int Read (void* pBuf, int nCount);
	int Write(void* pBuf, int nCount);

	/* Position */
	bool Seek(sint64 lOffset, SEEKPOS eseekpos);
	suint64 GetPosition();

	/* Length */
	suint64 GetLength();
	bool SetLength(const sint64& nLength);

	/*error*/
	void GetMMFLastError(sstring& strErr){ strErr = m_strErrMsg; }
};
