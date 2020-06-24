#pragma once

#include <windows.h>
#include <windowsx.h>
#include "global.h"
#include "ll.h"
#include "dlgdata.h"

#define isFocusTarget(designer) (!(designer)->listSelection)
#define isFocusControl(designer) ((designer)->listSelection && !(designer)->listSelection->next)
#define isFocusSelection(designer) ((designer)->listSelection && (designer)->listSelection->next)
#define isScaleFocus(designer) ((designer)->lHandle & ENABLE_ALL)

typedef enum {
	NO_TRACK = 0,
	TRACK_SELECTION,
	TRACK_SCALE,
	TRACK_CREATE,
	TRACK_MOVE
} TRACK_TYPE;

// All coordinates are in base window coordinate
typedef struct {
	HINSTANCE appInstance;
	HWND hwndMain;
	HWND hwndTarget;

	// Cursor
	HCURSOR curCurrent;
	HCURSOR curDefault;
	HCURSOR curMove;
	HCURSOR curLR;
	HCURSOR curUD;
	HCURSOR curNE;
	HCURSOR curSE;

	// DPI: WinAPI considers 96 DPI by default
	float dpiScaleX;
	float dpiScaleY;

	// Bounding offset of handles (default 6)
	int dd;

	// Image resources
	HBITMAP bmpHandleEnable;
	HBITMAP bmpHandleDisable;
	HBITMAP bmpMask;
	HDC hdcHandleEnable;
	HDC hdcHandleDisable;
	HDC hdcMask;
	LONG imgWidth;
	LONG imgHeight;
	BOOL bVisible;

	// Margin Box
	RECT rcMargin;    // Margin data, not position
	RECT rcMarginBox; // Position of margin box (base window coordinates)

	// Mouse Track & Capture
	TRACK_TYPE typeTrack;
	LONG lHandle;
	POINT ptTrackStart;
	RECT rcTrackPrev;
	RECT rcSelectionBB;
	RECT rcPreDragBB;
	BOOL bDetectHandleHover;

	// Controls and Selection
	DESIGNER_CONTROL_LIST* listControls;
	DESIGNER_SELECTION_LIST* listSelection;

	// Dialog Template
	// all detailed design data are originated from handles, preserved by the control list.
	DIALOG_TEMPLATE_DATA* pDlgTemplate;
} WINDOW_DESIGNER;

// core draw call
void InitWindowDesigner(WINDOW_DESIGNER*, HINSTANCE);
void ReleaseWindowDesigner(WINDOW_DESIGNER*);
BOOL DrawControlHandles(WINDOW_DESIGNER*, HDC, HWND, LONG);

// designer data
void DesignerAddControl(WINDOW_DESIGNER*, LPCWSTR, LPCWSTR, int, int, int, int);
void DesignerUpdateMarginBox(WINDOW_DESIGNER*);
void DesignerClearSelection(WINDOW_DESIGNER*);
void DesignerResetSelectionToFocus(WINDOW_DESIGNER*, HWND);
void DesignerAddToSelection(WINDOW_DESIGNER*, HWND);

// util
BOOL IsSubRect(const RECT*, const RECT*);
LONG IsHoveringOnHandles(WINDOW_DESIGNER*, POINT);
void MapWindowRectToDesigner(WINDOW_DESIGNER*, RECT*, HWND);

typedef struct {
	int nStatus;
	PBYTE pBase;
	size_t szData;
	size_t szOffset;
} DLG_TEMPLATE_BUILD_INFO;

// dialog data
int DesignerLaunchTemplate(WINDOW_DESIGNER*);
