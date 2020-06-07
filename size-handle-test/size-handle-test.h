#pragma once

#include "resource.h"

#define HANDLE_TOP_LEFT      0x01
#define HANDLE_TOP_CENTER    0x02
#define HANDLE_TOP_RIGHT     0x04
#define HANDLE_MIDDLE_LEFT   0x08
#define HANDLE_MIDDLE_RIGHT  0x10
#define HANDLE_BOTTOM_LEFT   0x20
#define HANDLE_BOTTOM_CENTER 0x40
#define HANDLE_BOTTOM_RIGHT  0x80

#define ENABLE_ALL           0xFF
#define ENABLE_VERTICES      0xA5 // TL|TR|BL|BR
#define ENABLE_EDGES         0x5A // TC|ML|MR|BC
#define ENABLE_RIGHTBOTTOM   0xD0 // MR|BC|BR

#define LOCK_ALL             0x00
#define LOCK_TOPLEFT         ENABLE_RIGHTBOTTOM
#define LOCK_VERTICES        ENABLE_EDGES
#define LOCK_EDGES           ENABLE_VERTICES

typedef struct {
	HWND hwndMain;
	HWND hwndTarget;

	// Image resources
	HBITMAP bmpHandleEnable;
	HBITMAP bmpHandleDisable;
	HBITMAP bmpMask;
	HGDIOBJ oldHandleEnable;
	HGDIOBJ oldHandleDisable;
	HGDIOBJ oldMask;
	HDC hdcHandleEnable;
	HDC hdcHandleDisable;
	HDC hdcMask;
	LONG imgWidth;
	LONG imgHeight;

	// Track rectangle
	HBITMAP bmpSnapshot;
	HPEN penTrackPen;
	HGDIOBJ oldSnapshot;
	HGDIOBJ oldTrackPen;
	HDC hdcTrack;
	POINT ptTrack;
	SIZE szTrack;
} WINDOW_DESIGNER;
