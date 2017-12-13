// D3D Sprite Tutorial
//
////////////////////////////////////////////////////////////////////////////////


#pragma warning( disable : 4996)

#pragma comment(linker, "/subsystem:windows")

#include <string>
#include <windows.h>
#include <mmsystem.h>

#include "GlcWinApp.h"

char		m_sCls[128]		;
HINSTANCE	m_hInst		= NULL;
HWND		m_hWnd		= NULL;
DWORD		m_dWinStyle	= WS_OVERLAPPEDWINDOW| WS_VISIBLE;
DWORD		m_dScnX		= 800;			// Screen Width
DWORD		m_dScnY		= 480;			// Screen Height
BOOL		m_bShowCusor= TRUE;			// Show Cusor

//Window+Device관련 함수들
INT		Create(HINSTANCE hInst);
INT		Run();
void	Cleanup();

LRESULT MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

const char*    m_fontFile1 ="_font/아리따-돋움(TTF)-SemiBold.ttf";
const char*    m_fontFile2 ="_font/Daum_Regular.ttf";
char    m_msg[] = "Hello world !!!! 안녕하세요. 1234567890";

glc::GdiFont*  font_gdi;
glc::GdiPFont* font_gdip;

void OnGdiPaint(HDC hdc);

INT Create( HINSTANCE hInst)
{
	// for gdiplus
	if(!glc::GdiPlus::getInstance()->isSuccess())
		return -1;

	font_gdip =glc::GdiPFont::create(m_fontFile1, 20.0F);
	if(!font_gdip)
		return -1;


	HRESULT ret = 0;
	font_gdi = glc::GdiFont::create(m_fontFile2, 20.0F);
	if(!font_gdi)
		return -1;

	m_hInst	= hInst;
	strcpy(m_sCls, "D3D Tutorial");

	WNDCLASS wc =								// Register the window class
	{
		CS_CLASSDC
		, WndProc
		, 0L
		, 0L
		, m_hInst
		, NULL
		, LoadCursor(NULL,IDC_ARROW)
		, (HBRUSH)GetStockObject(LTGRAY_BRUSH)
		, NULL
		, m_sCls
	};
	RegisterClass( &wc );

	RECT rc;									//Create the application's window

	SetRect( &rc, 0, 0, m_dScnX, m_dScnY);
	AdjustWindowRect( &rc, m_dWinStyle, FALSE );

	int iScnSysW = ::GetSystemMetrics(SM_CXSCREEN);
	int iScnSysH = ::GetSystemMetrics(SM_CYSCREEN);

	m_hWnd = CreateWindow( m_sCls
		, m_sCls
		, m_dWinStyle
		, (iScnSysW - (rc.right - rc.left))/2
		, (iScnSysH - (rc.bottom - rc.top))/2
		, (rc.right - rc.left)
		, (rc.bottom - rc.top)
		, GetDesktopWindow()
		, NULL
		, m_hInst
		, NULL );

	ShowWindow( m_hWnd, SW_SHOW );
	UpdateWindow( m_hWnd );
	::ShowCursor(m_bShowCusor);

	return 0;
}


void Cleanup()
{
	glc::GdiPFont::removeAllFontFamily();
	glc::GdiFont::removeAllHFontResource();

}


LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_KEYDOWN:
		{
			switch(wParam)
			{
				case VK_ESCAPE:
				{
					SendMessage(hWnd, WM_DESTROY, 0, 0);
					break;
				}
			}
			return 0;
		}
		case WM_DESTROY:
		{
			Cleanup();
			PostQuitMessage(0);
			return 0;
		}
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT ps;
			hdc=BeginPaint(hWnd, &ps);
			OnGdiPaint(hdc);
			EndPaint(hWnd, &ps);
			return 0;
		}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT Run()
{
	// Enter the message loop
	MSG msg={0};
	memset( &msg, 0, sizeof(msg) );

	while( msg.message!=WM_QUIT )
	{
		GetMessage(&msg, NULL, 0, 0);
		TranslateMessage( &msg );
		DispatchMessage( &msg );
		continue;

		//HDC hdc = GetDC(m_hWnd);
		//if(!hdc)
		//	continue;
		//OnGdiPaint(hdc);
		//ReleaseDC(m_hWnd, hdc);
	}
	UnregisterClass( m_sCls, m_hInst);

	glc::release();
	return 0;
}

LRESULT WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	return MsgProc(hWnd, msg, wParam, lParam);
}

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
	if(0>glc::initialize())
		return 0;
	if(FAILED(Create(hInst)))
		return 0;

	return Run();
}


void OnGdiPaint(HDC hdc)
{
	glc::GdiPlus::clearColor(XRGB(158, 137, 120));
	glc::GdiPlus::beginGraphics(hdc);

	if(font_gdi)
	{
		font_gdi->drawText(hdc, m_msg, 10, 20, RGB(253, 245, 232), RGB(114, 78, 51));
	}
	if(font_gdip)
	{
		font_gdip->drawPath(m_msg, 10.0F,  80.0F, XRGB(253,245,232));
		font_gdip->drawText(m_msg, 10.0F, 120.0F, XRGB(253,245,232));
	}

	glc::GdiPlus::endGraphics();
}
