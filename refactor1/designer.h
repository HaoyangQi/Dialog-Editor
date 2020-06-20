#pragma once

#include <windows.h>
#include <windowsx.h>
#include "global.h"
#include "ll.h"

#define isFocusTarget(designer) (!(designer)->listSelection)
#define isFocusControl(designer) ((designer)->listSelection && !(designer)->listSelection->next)
#define isFocusSelection(designer) ((designer)->listSelection && (designer)->listSelection->next)

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
	POINT ptTrackStart;
	RECT rcTrackPrev;
	RECT rcSelectionBB;
	BOOL bDetectHandleHover;

	// Controls and Selection
	DESIGNER_CONTROL_LIST* listControls;
	DESIGNER_SELECTION_LIST* listSelection;
} WINDOW_DESIGNER;

void InitWindowDesigner(WINDOW_DESIGNER*, HINSTANCE);
void ReleaseWindowDesigner(WINDOW_DESIGNER*);
BOOL DrawControlHandles(WINDOW_DESIGNER*, HDC, HWND, LONG);

void DesignerAddControl(WINDOW_DESIGNER*, LPCWSTR, LPCWSTR, int, int, int, int);
void DesignerUpdateMarginBox(WINDOW_DESIGNER*);
void DesignerClearSelection(WINDOW_DESIGNER*);
void DesignerResetSelectionToFocus(WINDOW_DESIGNER*, HWND);

LONG IsHoveringOnHandles(WINDOW_DESIGNER*, POINT);
void MapWindowRectToDesigner(WINDOW_DESIGNER*, RECT*, HWND);
