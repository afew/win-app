
#pragma comment(lib, "msimg32.lib")
#pragma warning(disable:4996)

#include <vector>
#include <string>

#include <Windows.h>
#include <stdio.h>


#pragma pack(push, 1)
struct TGA_Header
{
	BYTE    IDLength;
	BYTE    ColorMapType;
	BYTE    ImageType;
	WORD    CMapStart;
	WORD    CMapLength;
	BYTE    CMapDepth;
	WORD    XOffset;
	WORD    YOffset;
	WORD    Width;
	WORD    Height;
	BYTE    PixelDepth;
	BYTE    ImageDescriptor;
};

#pragma pack(pop)

HBITMAP Load32bppTga(const TCHAR * pFileName, bool bPreMultiply)
{
	FILE* fp = fopen(pFileName, "rb");
	if(!fp)
		return NULL;

	TGA_Header header;

	size_t v_read = 0;
	v_read = fread(&header, sizeof(header), 1, fp);

	if((header.IDLength!=0) || (header.ColorMapType!=0) || (header.ImageType!=2) ||
		 (header.PixelDepth!=32) || (header.ImageDescriptor!=8))
	{
		fclose(fp);
		return NULL;
	}

	BITMAPINFO bmp ={{sizeof(BITMAPINFOHEADER), header.Width, header.Height, 1, 32}};
	BYTE* pBits = NULL;

	HBITMAP hBmp = CreateDIBSection(NULL, & bmp, DIB_RGB_COLORS, (void**)&pBits, NULL, NULL);
	if(hBmp==NULL)
	{
		fclose(fp);
		return NULL;
	}

	v_read = fread(pBits, sizeof(char), header.Width * header.Height * 4, fp);
	fclose(fp);


	if(bPreMultiply)
	{
		for(int y=0; y<header.Height; y++)
		{
			BYTE * pPixel = (BYTE *)pBits + header.Width * 4 * y;

			for(int x=0; x<header.Width; x++)
			{
				pPixel[0] = pPixel[0] * pPixel[3] / 255;
				pPixel[1] = pPixel[1] * pPixel[3] / 255;
				pPixel[2] = pPixel[2] * pPixel[3] / 255;

				pPixel += 4;
			}
		}
	}

	return hBmp;
}

void AlphaDraw(HDC hDC, int x, int y, int width, int height, HBITMAP hBmp)
{
	HDC     hMemDC = ::CreateCompatibleDC(hDC);
	HGDIOBJ hOld   = ::SelectObject(hMemDC, hBmp);


	HBRUSH hBrush = ::CreateSolidBrush(RGB(0xFF, 0xFF, 0));
	::SelectObject(hDC, hBrush);
	::Ellipse(hDC, x, y, width*2, height * 2);
	::SelectObject(hDC, GetStockObject(WHITE_BRUSH));
	::DeleteObject(hBrush);

	::BitBlt(hDC, x, y, width, height, hMemDC, 0, 0, SRCCOPY);

	BLENDFUNCTION pixelblend ={AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

	::AlphaBlend(hDC, x, y+height, width, height, hMemDC, 0, 0, width, height, pixelblend);

	BLENDFUNCTION blend50 ={AC_SRC_OVER, 0, 128, 0};

	::AlphaBlend(hDC, x+width, y, width, height, hMemDC, 0, 0, width, height, blend50);

	::SelectObject(hMemDC, hOld);
	::DeleteObject(hMemDC);
}

void OnDraw(HDC hDC, const char * pFileName)
{
	static HBITMAP hBmp = NULL;

	if(hBmp==NULL)
		hBmp = Load32bppTga(pFileName, true);

	BITMAP bmp;
	GetObject(hBmp, sizeof(BITMAP), & bmp);

	AlphaDraw(hDC, 10, 10, bmp.bmWidth, bmp.bmHeight, hBmp);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	switch(message)
	{
		case WM_PAINT:
			OnDraw(BeginPaint(hWnd, &ps), "alpha_mask2.tga"); // szFileName);
			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


static std::string g_app_name = "AlphaBlending";
static HWND        g_app_win  = NULL;

int main(int, char**)
{
	WNDCLASS wc={0};
	wc.style		= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	= (WNDPROC)WndProc;
	wc.hInstance	= (HINSTANCE)GetModuleHandle(NULL);
	wc.hIcon		= LoadIcon(NULL, IDC_ICON);
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground= (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName= g_app_name.c_str();

	if(!RegisterClass(&wc))
		return 0;

	g_app_win = CreateWindow(g_app_name.c_str(), g_app_name.c_str(), WS_OVERLAPPEDWINDOW
							, CW_USEDEFAULT, CW_USEDEFAULT, 800, 800
							, NULL, NULL
							, wc.hInstance, NULL);

	if(!g_app_win)
	{
		return 0;
	}

	ShowWindow(g_app_win, SW_SHOW);
	UpdateWindow(g_app_win);

	MSG msg{};
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(WM_QUIT == msg.message)
			break;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

