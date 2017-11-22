
#ifdef _MSC_VER
#pragma once
#endif

#ifndef _GlcWinApp_H_
#define _GlcWinApp_H_

#include <string>
#include <tuple>
#include <map>
#include <windows.h>
#include <CommCtrl.h>

#ifndef SAFE_DELETE
  #define SAFE_DELETE(p)       do{ if(p) { delete (p);     (p)=NULL; } }while(0);
#endif
#ifndef SAFE_DELETE_ARRAY
  #define SAFE_DELETE_ARRAY(p) do{ if(p) { delete[] (p);   (p)=NULL; } }while(0);
#endif
#ifndef SAFE_RELEASE
  #define SAFE_RELEASE(p)      do{ if(p) { (p)->Release(); (p)=NULL; } }while(0);
#endif

#ifndef GLC_BEGIN
  #define GLC_BEGIN namespace glc {
#endif
#ifndef GLC_END
  #define GLC_END }
#endif

#ifndef RGBA
  #define RGBA(a,r,g,b) ( ((UINT)(r)<< 0) & 0x000000FF | \
						  ((UINT)(g)<< 8) & 0x0000FF00 | \
						  ((UINT)(b)<<16) & 0x00FF0000 | \
						  ((UINT)(a)<<24) & 0x00000000 )
#endif
#ifndef ARGB
  #define ARGB(a,r,g,b) ( ((UINT)(b)<< 0) & 0x000000FF | \
						  ((UINT)(g)<< 8) & 0x0000FF00 | \
						  ((UINT)(r)<<16) & 0x00FF0000 | \
						  ((UINT)(a)<<24) & 0xFF000000 )
#endif
#ifndef XRGB
  #define XRGB(r,g,b)   ( ((UINT)(b)<< 0) & 0x000000FF | \
						  ((UINT)(g)<< 8) & 0x0000FF00 | \
						  ((UINT)(r)<<16) & 0x00FF0000 | 0xFF000000 )
#endif

GLC_BEGIN

struct RECT_L
{
	int x, y, w, h;
};

enum
{
	L_FAIL		= -1,
	L_OK		= 0,
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int          initialize();
int          release();

::HIMAGELIST createImageList(UINT uId, int cx);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

struct TWIN_PARAM
{
    LPCSTR    className;
    RECT_L    rc;
	DWORD     dStyle  ={};
    DWORD     exStyle ={};
	HWND      parent  ={};
    HMENU     menu    ={};
    void*     param   ={};
	TWIN_PARAM(LPCSTR clzz, const RECT_L& _rc
				, DWORD style=0, DWORD estyle=0,HWND prn=NULL, HMENU m=NULL, void* p=NULL) 
				: className(clzz), rc(_rc), parent(prn), menu(m), param(p){}
};

struct TWIN_MSG
{
    HWND        hwnd;
    UINT        message;
    WPARAM      wParam;
    LPARAM      lParam;
};

class IWinBase
{
protected:
	HWND			m_hWnd{};		// this
	HWND			m_hPrn{};		// parent
	std::string		m_clzz;			// clazz
	std::string		m_name;			// name
public:
	IWinBase();
	virtual ~IWinBase();

	HWND hwnd()  const { return m_hWnd; }
	HWND parent() const { return m_hPrn; }
	void parent(HWND p) { m_hPrn = p;    }

	virtual INT		create(const TWIN_PARAM&);
	virtual LRESULT msgPrc(const TWIN_MSG&);
	static INT Run();
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class WinControlList : public IWinBase
{
protected:
	int m_asc;

public:
	static WinControlList* create(const char* name, const RECT_L& rc, HWND parent=NULL);
	WinControlList();
	virtual ~WinControlList();
	virtual LRESULT msgPrc(HWND, UINT, WPARAM, LPARAM);

	int   imageList(void* handle, int image_list);
	int   insertColumn(int index, const char* name, int width=150);
	int   insertItem(int index, void* param, const char* name, int image);
	int   itemText(int index, int column, const char* name);
	void* item(int index);								// get item pointer
	std::string itemText(int index, int column=0);		// get item text
protected:
	int  Init(const char*name, const RECT_L& rc, HWND parent=NULL);
	void Destroy();
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class GdiFont
{
public:
	GdiFont();
	virtual ~GdiFont();

	static int   createLogFont(const char* fontFileName, LOGFONTA* logFont);
	static HFONT createHFont(float floatHeight, const char* fontFileName);
	static int   removeAllHFontResource();

	static GdiFont* create(const char* font_file, float font_height);

	void	drawText(HDC hdc, const char* msg, int x, int y, UINT color=0xFFFFFF, UINT color_shadow=0x000000);
	HFONT	font() const { return m_font; }
	float	height() const { return m_height; }
protected:
	HFONT	m_font;
	float	m_height;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class GdiPlus
{
protected:
	unsigned long long m_token;
	bool m_bSuccess;

public:
	static GdiPlus* getInstance();

	GdiPlus();
	~GdiPlus();
	bool isSuccess() const { return m_bSuccess; }

	static void beginGraphics(HDC hdc);
	static void endGraphics();
	static void clearColor(UINT color=ARGB(0xFF, 0xFF, 0xFF, 0xFF));
};

class GdiPFont
{
public:
	GdiPFont();
	virtual ~GdiPFont();

	static int createFamily(const char* fontFileName);
	static int removeAllFontFamily();

	static int drawPath  (std::string msg, float pos_x, float pos_y, const char* fontFileName, float height, UINT color=XRGB(0, 0, 0));
	static int drawString(std::string msg, float pos_x, float pos_y, const char* fontFileName, float height, UINT color=XRGB(0, 0, 0));

	static GdiPFont* create(const char* font_file, float font_height);

	void	drawText(const char* msg, float pos_x, float pos_y, UINT color=0xFFFFFFFF);
	void	drawPath(const char* msg, float pos_x, float pos_y, UINT color=0xFFFFFFFF);
	float	height() const { return m_height; }
protected:
	float		m_height;
	std::string	m_file;
};

GLC_END

#endif
