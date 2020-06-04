#pragma once

#include "resource.h"

typedef struct {
	RECT handle;
	BOOL enable;
} SIZE_BOX_DATA;

// Index map of 8 control points
typedef enum {
	POS_TOP_LEFT_HANDLE      = 0,
	POS_TOP_CENTER_HANDLE    = 1,
	POS_TOP_RIGHT_HANDLE     = 2,
	POS_MIDDLE_LEFT_HANDLE   = 3,
	POS_MIDDLE_RIGHT_HANDLE  = 4,
	POS_BOTTOM_LEFT_HANDLE   = 5,
	POS_BOTTOM_CENTER_HANDLE = 6,
	POS_BOTTOM_RIGHT_HANDLE  = 7,
	POS_INSIDE               = 8,
} DRAG_HANDLE_POSITION;

BOOL InitSizeBoxInstance(HINSTANCE hInstance, HWND hParent, int nCmdShow);
