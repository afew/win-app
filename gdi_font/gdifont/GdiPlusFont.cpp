
#pragma comment(lib, "Gdiplus.lib")

#include "GlcWinApp.h"
#include <gdiplus.h>

GLC_BEGIN

static std::wstring ToWideString(const char* str)
{
	int wcs_len =
	MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, NULL);
	std::wstring ret_wstr(wcs_len-1, 0);
	MultiByteToWideChar(CP_ACP, 0, str, -1, &ret_wstr[0], wcs_len);
	return ret_wstr;
}

static std::string ToMultiByteString(const wchar_t* w_str)
{
	int str_len =
	WideCharToMultiByte(CP_ACP, 0, w_str, -1, NULL, 0, NULL, NULL);
	std::string ret_str(str_len-1, 0);
	WideCharToMultiByte(CP_ACP, 0, w_str, -1, &ret_str[0], str_len, NULL, NULL);
	return ret_str;
}

static std::string FullPath(const char* retPath)
{
	char path_t[8192+4]={0};
	int ren = GetFullPathName(retPath, 8192, path_t, NULL);
	std::string ret_str = path_t;
	return ret_str;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static GdiPlus                             gdiplus_starter;
static Gdiplus::PrivateFontCollection*             gdiplus_font_collection = NULL;
static std::map<std::string, Gdiplus::FontFamily*> gdiplus_font_family;
static Gdiplus::Graphics*                          gdiplus_graphics = NULL;
static Gdiplus::ARGB                               gdiplus_clear_color = 0xFFFFFFFF;

GdiPlus::GdiPlus(): m_token(0), m_bSuccess(0)
{
	Gdiplus::GdiplusStartupInput gpsi;
	m_bSuccess = Gdiplus::GdiplusStartup((ULONG_PTR*)&m_token, &gpsi, NULL) == Gdiplus::Ok;
	if(m_bSuccess)
		gdiplus_font_collection = new Gdiplus::PrivateFontCollection;
}

GdiPlus::~GdiPlus()
{
	if(gdiplus_font_collection)
	{
		delete (Gdiplus::PrivateFontCollection*)gdiplus_font_collection;
		gdiplus_font_collection = NULL;
	}
	Gdiplus::GdiplusShutdown((ULONG_PTR)m_token);
}
GdiPlus* GdiPlus::getInstance() { return &gdiplus_starter; }

void GdiPlus::beginGraphics(HDC hdc)
{
	gdiplus_graphics = new Gdiplus::Graphics(hdc);
	gdiplus_graphics->Clear(gdiplus_clear_color);
	gdiplus_graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	gdiplus_graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
}
void GdiPlus::endGraphics()
{
	if(gdiplus_graphics)
	{
		delete gdiplus_graphics;
	}
}

void GdiPlus::clearColor(UINT color/*=MakeGdiColor(0xFF, 0xFF, 0xFF, 0xFF)*/)
{
	gdiplus_clear_color = color;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

GdiPFont::GdiPFont()
	: m_height(0.0)
{
}

GdiPFont::~GdiPFont()
{
}

int GdiPFont::createFamily(const char* fontFileName)
{
	HRESULT ret = 0;
	// find
	std::map<std::string, Gdiplus::FontFamily*>::iterator find_it = gdiplus_font_family.find(std::string(fontFileName));
	if(find_it != gdiplus_font_family.end())
	{
		return 0;
	}

	std::string full_path = FullPath(fontFileName);
	std::wstring wstr_file = ToWideString(full_path.c_str());

	ret = gdiplus_font_collection->AddFontFile(wstr_file.c_str());
	int count = gdiplus_font_collection->GetFamilyCount();
	if(0>=count)
		return NULL;

	int found_count = 0;
	Gdiplus::FontFamily* fontFamily = new Gdiplus::FontFamily[count];
	if(NULL == fontFamily)
		return NULL;

	gdiplus_font_collection->GetFamilies(count, fontFamily, &found_count);
	gdiplus_font_family.insert(std::pair<std::string, Gdiplus::FontFamily*>(std::string(fontFileName), fontFamily));
	return 0;
}

int GdiPFont::removeAllFontFamily()
{
	if(gdiplus_font_family.empty())
		return 0;

	for(std::map<std::string, Gdiplus::FontFamily*>::iterator it = gdiplus_font_family.begin(); it != gdiplus_font_family.end(); ++it)
	{
		const std::string& fontFileName = it->first;
		Gdiplus::FontFamily* font_family = it->second;
		delete [] font_family;
	}
	gdiplus_font_family.clear();
	return 0;
}

int GdiPFont::drawPath(std::string msg, float pos_x, float pos_y, const char* fontFileName, float height, UINT color)
{
	height = float(height * 96.0/ 72.0);
	// find
	std::map<std::string, Gdiplus::FontFamily*>::iterator find_it = gdiplus_font_family.find(std::string(fontFileName));
	Gdiplus::FontFamily* font_family = NULL;
	if(find_it != gdiplus_font_family.end())
	{
		font_family = find_it->second;
	}
	if(NULL == font_family)
		return -1;

	std::wstring wstr_msg = ToWideString(msg.c_str());
	Gdiplus::StringFormat strformat;
	Gdiplus::GraphicsPath path;

	path.AddString(wstr_msg.c_str(), wcslen(wstr_msg.c_str()), font_family,
	Gdiplus::FontStyleRegular, height, Gdiplus::PointF(pos_x, pos_y), &strformat);
	//{
	//	Gdiplus::Pen pen(Gdiplus::Color(80, 114, 78, 51), 4.0f);
	//	pen.SetLineJoin(Gdiplus::LineJoinRound);
	//	gdiplus_graphics->DrawPath(&pen, &path);
	//}
	//{
	//	Gdiplus::Pen pen(Gdiplus::Color(160, 114, 78, 51), 2.0f);
	//	pen.SetLineJoin(Gdiplus::LineJoinRound);
	//	gdiplus_graphics->DrawPath(&pen, &path);
	//}
	{
		Gdiplus::ARGB argb = color;
		Gdiplus::SolidBrush brush(argb);
		gdiplus_graphics->FillPath(&brush, &path);
	}

	return 0;
}

int GdiPFont::drawString(std::string msg, float pos_x, float pos_y, const char* fontFileName, float height, UINT color)
{
	// find
	std::map<std::string, Gdiplus::FontFamily*>::iterator find_it = gdiplus_font_family.find(std::string(fontFileName));
	Gdiplus::FontFamily* font_family = NULL;
	if(find_it != gdiplus_font_family.end())
	{
		font_family = find_it->second;
	}
	if(NULL == font_family)
		return -1;

	std::wstring wstr_msg = ToWideString(msg.c_str());
	Gdiplus::StringFormat strformat;
	{
		Gdiplus::ARGB argb = color;
		Gdiplus::Font font(font_family, height, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
		Gdiplus::SolidBrush brush(argb);
		gdiplus_graphics->DrawString(wstr_msg.c_str(), -1, &font, Gdiplus::PointF(pos_x, pos_y), &strformat, &brush);
	}

	return 0;
}

//UINT GdiPFont::makeGdiColor(BYTE r, BYTE g, BYTE b, BYTE a)
//{
//	Gdiplus::ARGB ret = Gdiplus::Color::MakeARGB(a, r, g, b);
//	return ret;
//}

GdiPFont* GdiPFont::create(const char* font_file, float font_height)
{
	if(0>GdiPFont::createFamily(font_file))
		return NULL;

	GdiPFont* ret = new GdiPFont;
	ret->m_height = font_height;
	ret->m_file   = font_file;
	return ret;
}

void GdiPFont::drawText(const char* msg, float pos_x, float pos_y, UINT color)
{
	glc::GdiPFont::drawString(std::string(msg), pos_x, pos_y, m_file.c_str(), m_height, color);
}

void GdiPFont::drawPath(const char* msg, float pos_x, float pos_y, UINT color)
{
	glc::GdiPFont::drawPath(std::string(msg), pos_x, pos_y, m_file.c_str(), m_height, color);
}


GLC_END

