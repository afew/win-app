
#pragma comment(lib, "ComCtl32.lib")

#include "GlcWinApp.h"

GLC_BEGIN

typedef std::map<HWND, LPVOID>	mpWINOBJ;

static mpWINOBJ mp_obj_win;

static LPVOID FindObjectWindow(HWND hWnd);
static INT    InsertObjectWindow(HWND hWnd, LPVOID obj);
static void   DeleteObjectWindow(HWND hWnd);

static LRESULT WINAPI WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg)
	{
		case WM_CREATE:
		{
			CREATESTRUCT* st = (CREATESTRUCT*)lParam;
			IWinBase* win_obj = NULL;
			if(st && st->lpCreateParams)
			{
				win_obj = (IWinBase*)st->lpCreateParams;
				InsertObjectWindow(hWnd, win_obj);
			}
			break;
		}
		case WM_DESTROY:
		{
			IWinBase* win_app = (IWinBase*)FindObjectWindow(hWnd);
			if(win_app)
			{
				DeleteObjectWindow(hWnd);
				return win_app->msgPrc({hWnd, uMsg, wParam, lParam});
			}
			return DefWindowProc( hWnd, uMsg, wParam, lParam );
		}
	}
	IWinBase* win_app = (IWinBase*)FindObjectWindow(hWnd);
	if(!win_app)
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	return win_app->msgPrc({hWnd, uMsg, wParam, lParam});
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int initialize()
{
	if(!glc::GdiPlus::getInstance()->isSuccess())
		return -1;
	return 0;
}

int release()
{
	GdiFont::removeAllHFontResource();
	GdiPFont::removeAllFontFamily();
	return 0;
}


::HIMAGELIST createImageList(UINT uId, int cx)
{
	::HIMAGELIST ret = ::ImageList_LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(uId), cx, 1, RGB(255,255,255));
	return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

IWinBase::IWinBase()
{
}


IWinBase::~IWinBase()
{
	if(!m_hWnd)
		return;
	DestroyWindow(m_hWnd);
	m_hWnd = 0;
}

LRESULT IWinBase::msgPrc(const TWIN_MSG& msg)
{
	return ::DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
}

INT IWinBase::create(const TWIN_PARAM& param)
{
	m_clzz = param.className;

	WNDCLASS wc{0};
	wc.style         = CS_CLASSDC | CS_DBLCLKS;
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = (HINSTANCE)GetModuleHandle(NULL);
	wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = m_clzz.c_str();

	WNDCLASS wc_stored;
	if(::GetClassInfo(wc.hInstance, wc.lpszClassName, &wc_stored))
	{
		if(wc_stored.lpfnWndProc != wc.lpfnWndProc)
		{
			MessageBox(NULL, "Already Regist, but not equal to windows proc", "Error", MB_OK);
			return -1;
		}
	}
	else
	{
		if(!RegisterClass(&wc))
		{
			DWORD err  = GetLastError();
			if(ERROR_CLASS_ALREADY_EXISTS != err)
			{
				MessageBox(NULL, "Can`t register Window", "Error", MB_OK);
				return -1;
			}
		}
	}


	RECT rc={};									//create the application's window
	DWORD wStyle	= param.dStyle | WS_OVERLAPPED| WS_CAPTION| WS_SYSMENU| WS_VISIBLE;
	INT iScnSysW = ::GetSystemMetrics(SM_CXSCREEN);
	INT iScnSysH = ::GetSystemMetrics(SM_CYSCREEN);
	INT pos_x = param.rc.x;
	INT pos_y = param.rc.y;

	if(-1 == param.rc.x)
		pos_x = (iScnSysW - (rc.right-rc.left))/2;
	if(-1 == pos_y)
		pos_y = (iScnSysH - (rc.bottom-rc.top))/2;

	rc = {0, 0, param.rc.w, param.rc.h};
	::AdjustWindowRect( &rc, wStyle, FALSE );

	m_hWnd = ::CreateWindow( m_clzz.c_str()
		, m_clzz.c_str()
		, wStyle
		, pos_x
		, pos_y
		, (rc.right  - rc.left)
		, (rc.bottom - rc.top)
		, NULL
		, NULL
		, wc.hInstance
		, this);


	::ShowWindow( m_hWnd, SW_SHOW );
	::UpdateWindow( m_hWnd );
	::ShowCursor(TRUE);

	return 0;
}

INT IWinBase::Run()
{
	MSG msg{};
	while( msg.message!=WM_QUIT )
	{
		if( PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE ) )
		{
			if(GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		else
		{
			if(mp_obj_win.empty())
				PostQuitMessage(0);
		}
	}

	//UnregisterClass( m_clzz, (HINSTANCE)GetModuleHandle(0) );
	return 0;
}



LPVOID FindObjectWindow(HWND hWnd)
{
	auto it = mp_obj_win.find(hWnd);
	if(it == mp_obj_win.end())
		return NULL;

	return it->second;
}

INT InsertObjectWindow(HWND hWnd, LPVOID obj)
{
	auto old_obj = FindObjectWindow(hWnd);
	if(old_obj)
		return -1;

	mp_obj_win.insert(mpWINOBJ::value_type(hWnd, obj));
	return 0;
}

void DeleteObjectWindow(HWND hWnd)
{
	auto it = mp_obj_win.find(hWnd);
	if(it == mp_obj_win.end())
		return;

	mp_obj_win.erase(it);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

WinControlList::WinControlList()
 : m_asc(1)
{
}


WinControlList::~WinControlList()
{
	int count = ListView_GetItemCount(m_hWnd);

	for(int index=0; index<count; ++index)
	{
		LV_ITEM item ={0};
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.iItem = index;
		ListView_GetItem(m_hWnd, &item);

		int cc;
		cc = 0;
	}

}

WinControlList* WinControlList::create(const char*name, const RECT_L& rc, HWND parent)
{
	WinControlList* ret = new WinControlList;
	if(0>ret->Init(name, rc, parent))
	{
		delete ret;
		return NULL;
	}


	return ret;
}

int WinControlList::Init(const char*name, const RECT_L& rc, HWND parent)
{
	// 리스트 컨트롤을 자세히 보기로 만든다.
	m_clzz = (const char*)WC_LISTVIEWA;
	if(name)
		m_name = name;
	m_hWnd = ::CreateWindow(m_clzz.c_str()
							, name
							, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS
							, rc.x, rc.y, rc.w, rc.h
							, parent, NULL, (HINSTANCE)GetModuleHandle(NULL), NULL);
	if(!m_hWnd)
		return -1;

	ListView_SetExtendedListViewStyle( m_hWnd, LVS_EX_FULLROWSELECT  | LVS_EX_GRIDLINES);
	return 0;
}

int WinControlList::imageList(void* handle, int image_list)
{
	if(!ListView_SetImageList(m_hWnd, (HIMAGELIST)handle, image_list))
		return -1;
	return 0;
}

int WinControlList::insertColumn(int index, const char* name, int width)
{
	LVCOLUMN COL= {};
	COL.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT |LVCF_SUBITEM;
	COL.fmt     = LVCFMT_LEFT;
	COL.cx      = width;
	COL.pszText = (LPSTR)name;
	COL.iSubItem= index;
	ListView_InsertColumn(m_hWnd, index, &COL);

	return 0;
}

int WinControlList::insertItem(int index, void* param, const char* name, int image)
{
	LV_ITEM item  = {0};
	item.mask     = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	item.lParam   = (LPARAM)param;
	item.pszText  = (LPSTR)name;
	item.iItem    = index;
	item.iImage   = image;
	int ret = ListView_InsertItem(m_hWnd, &item);
	return ret;
}

int WinControlList::itemText(int index, int column, const char* name)
{
	ListView_SetItemText(m_hWnd, index, column, (LPSTR)name);
	return L_OK;
}

std::string WinControlList::itemText(int index, int column/*=0*/)
{
	char str[4092+4]={0};
	std::string ret;
	LV_ITEM item ={0};
	item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	item.iItem = index;
	if(!ListView_GetItem(m_hWnd, &item))
		return ret;

	ListView_GetItemText(m_hWnd, index, column, str, 4092);
	ret = str;
	return ret;
}

void* WinControlList::item(int index)
{
	char str[4092+4]={0};
	std::string ret;
	LV_ITEM item ={0};
	item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	item.iItem = index;
	if(!ListView_GetItem(m_hWnd, &item))
		return NULL;
	return (void*)item.lParam;
}

static int CALLBACK Compare(LPARAM p1, LPARAM p2, LPARAM lParamSort);

LRESULT WinControlList::msgPrc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPNMLISTVIEW pNM=(LPNMLISTVIEW)lParam;
	NMLVCUSTOMDRAW* pNMCustomDraw = (NMLVCUSTOMDRAW*)(pNM);

	if(WM_NOTIFY != uMsg)
		return 1;

	if(pNM->hdr.hwndFrom != m_hWnd)
		return 1;

	UINT code = pNM->hdr.code;
	switch(code)
	{
		case LVN_GETDISPINFO:
		{
			return 0;
		}

		case NM_CUSTOMDRAW:
		{
			switch(pNMCustomDraw->nmcd.dwDrawStage)
			{
				case CDDS_PREPAINT:
				{
					return CDRF_NOTIFYITEMDRAW;
				}

				case CDDS_POSTPAINT:
				{
					return CDRF_DODEFAULT;
				}

				case CDDS_ITEMPREPAINT:
				{
					{
						pNMCustomDraw->clrText   = RGB(0,0,255);
						pNMCustomDraw->clrTextBk = RGB(255,255,255);
					}

					return CDRF_NEWFONT;
				}
			}
			return CDRF_DODEFAULT;
			break;
		}

		case NM_CUSTOMDRAW+100:
		{
			switch(pNMCustomDraw->nmcd.dwDrawStage)
			{
				case CDDS_ITEM:
				{
					pNMCustomDraw->clrText = RGB(0, 0, 200);
					break;
				}
				default:
					::SetWindowLong(::GetParent(m_hWnd), DWL_MSGRESULT, CDRF_DODEFAULT);
					break;
			}
		}break;

		case LVN_COLUMNCLICK:
		{
			this->m_asc = (1==m_asc)? -1 : 1;
			struct A_T { HWND h; int c; int asc; } param = {m_hWnd, pNM->iSubItem, this->m_asc};
			ListView_SortItems(m_hWnd, Compare, (LPARAM)&param);
			return FALSE;
		}

		case LVN_ITEMCHANGED:
		{
			INT index = pNM->iItem;
			if(ListView_GetItemState(m_hWnd, index, LVIS_SELECTED) & LVIS_SELECTED)
			{
			}
		}
	}

	return 1;
}

int CALLBACK Compare(LPARAM p1, LPARAM p2, LPARAM lParamSort)
{
	struct A_T { HWND h; int c; int asc; } *param = (A_T*)lParamSort;
	int ret;
	int idx1, idx2;
	char str1[256+4]={0},str2[256+4]={0};
	LVFINDINFO fi ={0};

	// p1으로부터 첫 번째 항목의 인덱스를 찾음
	fi.flags=LVFI_STRING;
	fi.psz=(LPCTSTR)p1;
	fi.vkDirection=VK_DOWN;
	idx1=ListView_FindItem(param->h,-1,&fi);

	// p2로부터 두 번째 항목의 인덱스를 찾음
	fi.psz=(LPCTSTR)p2;
	idx2=ListView_FindItem(param->h,-1,&fi);

	// 두 항목의 비교 텍스트를 구한다.
	ListView_GetItemText(param->h, idx1, param->c, str1, 256);
	ListView_GetItemText(param->h, idx2, param->c, str2, 256);

	ret=lstrcmp(str1,str2);
	ret *= param->asc;
	return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


GLC_END
