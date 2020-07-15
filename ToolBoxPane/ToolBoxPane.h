#pragma once

#include "framework.h"
#include "resource.h"
#include <windowsx.h>
#include <commctrl.h>

#define isCategory(pvi) ((pvi)->next && (pvi)->next->nLevel > (pvi)->nLevel)
#define isChild(pvi) ((pvi)->parent)
#define hasItemImage(pvi) ((pvi)->nImageIndex != -1)
#define getTextMargin(pvi) (hasItemImage(pvi) ? ptp->dmItemTextMargin : 0)
#define measureImageWidth(ptp) ((ptp)->dmItemHeight - (ptp)->dmItemImageMargin * 2)
#define ITEM_NO_PARENT (-1)
#define ID_NO_RES (0)

#define DebugPrintf(str, ...) {\
	wchar_t buf[256] = L"\0";\
	swprintf_s(buf, 256, str, __VA_ARGS__);\
	OutputDebugString(buf); }

typedef struct view_list_item {
	// resources
	WCHAR* strContent;
	// properties
	LONG id;
	LONG nLevel;                  // root == level 0
	int nImageIndex;
	RECT rcItemText;              // BB of item, but starts from text (skip indention)
	COLORREF clrText;
	BOOL bCollapse;               // expansion status; for non-category item, this field is ignored
	BOOL bSelect;
	BOOL bHover;

	struct view_list_item* next;  // by working with nLevel, tree structure can be expressed linearly
	struct view_list_item* parent;
} VIEW_ITEM;

typedef struct {
	// resources
	HINSTANCE appInstance;
	HWND hwnd;
	HFONT fontRoot;
	HFONT fontChild;
	HFONT fontOld;
	HIMAGELIST hStateImageList;
	HIMAGELIST hItemImageList;
	HBRUSH brBackground;
	// properties
	BOOL bMouseTrack;
	BOOL bToggleList;       // if TRUE, category can be expanded/collapsed
	SIZE szPane;
	LONG dmItemHeight;
	LONG dmItemImageMargin; // how much imgage is scaled with repect to the BB
	LONG dmItemTextMargin;  // additional space between text and image BB
	LONG dmLeftMargin;      // only applies to an empty level 0 item
	//LONG dmCategoryImageReserveWidth;
	COLORREF clrForeground;
	COLORREF clrHover;
	COLORREF clrSelect;
	// data structures
	SCROLLINFO si;
	TRACKMOUSEEVENT tme;
	VIEW_ITEM* pvi;
	VIEW_ITEM* itemSelect;
	VIEW_ITEM* itemHover;
} TOOLBOX_PANE;

typedef struct {
	LPCWSTR name;
	WORD idRes;
} TOOLBOX_CONTENT;

void InitToolBoxPane(TOOLBOX_PANE* ptp, HWND hwnd, HINSTANCE hInst);
void ReleaseToolBoxPane(TOOLBOX_PANE* ptp);
int ToolBoxAddImage(HIMAGELIST hImgList, HBITMAP bmp, COLORREF clrTransparent);
VIEW_ITEM* ItemAppend(TOOLBOX_PANE* ptp, VIEW_ITEM* pvi, LONG idParent, LONG id, LPCWSTR content, LPCWSTR strImage);
void ReleaseItemList(VIEW_ITEM* pvi);
VIEW_ITEM* ToolboxBindViewData(TOOLBOX_PANE* ptp, VIEW_ITEM* pvi, VIEW_ITEM* selectionDefault);
VIEW_ITEM* ItemGetNext(VIEW_ITEM* current);

void OnSize(TOOLBOX_PANE* ptp, LONG w, LONG h);
void OnScroll(TOOLBOX_PANE* ptp, int scrollType, int lineStep);
void OnPaint(TOOLBOX_PANE* ptp, HDC hdc, LPRECT rcUpdate, BOOL bErase);
void OnMouseMove(TOOLBOX_PANE* ptp, int x, int y);
void OnMouseLeave(TOOLBOX_PANE* ptp);
void OnMouseLeftButtonPress(TOOLBOX_PANE* ptp, int x, int y);
