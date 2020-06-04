#pragma once

#include "resource.h"

typedef struct {
	RECT handle;
	BOOL enable;
} SIZE_BOX_DATA;

// Index map of 8 control points
#define POS_TOP_LEFT_HANDLE      0
#define POS_TOP_CENTER_HANDLE    1
#define POS_TOP_RIGHT_HANDLE     2
#define POS_MIDDLE_LEFT_HANDLE   3
#define POS_MIDDLE_RIGHT_HANDLE  4
#define POS_BOTTOM_LEFT_HANDLE   5
#define POS_BOTTOM_CENTER_HANDLE 6
#define POS_BOTTOM_RIGHT_HANDLE  7
// additional status number for inside size box
#define POS_INSIDE               -1

BOOL InitSizeBoxInstance(HINSTANCE hInstance, HWND hParent, int nCmdShow);
