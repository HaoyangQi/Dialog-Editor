#pragma once

#include "resource.h"
#include "designer.h"
#include <stdio.h>

#define MAX_LOADSTRING 100
#define DEFAULT_TARGET_X 7
#define DEFAULT_TARGET_Y 7
#define DEFAULT_TARGET_WIDTH 500
#define DEFAULT_TARGET_HEIGHT 300

#define HANDLE_TOP_LEFT      0x01
#define HANDLE_TOP_CENTER    0x02
#define HANDLE_TOP_RIGHT     0x04
#define HANDLE_MIDDLE_LEFT   0x08
#define HANDLE_MIDDLE_RIGHT  0x10
#define HANDLE_BOTTOM_LEFT   0x20
#define HANDLE_BOTTOM_CENTER 0x40
#define HANDLE_BOTTOM_RIGHT  0x80
// dummy bits, flag only
#define HANDLE_INSIDE        0x100
#define HANDLE_NOWHERE       0x200

#define ENABLE_ALL           0xFF
#define ENABLE_VERTICES      0xA5 // TL|TR|BL|BR
#define ENABLE_EDGES         0x5A // TC|ML|MR|BC
#define ENABLE_RIGHTBOTTOM   0xD0 // MR|BC|BR

#define LOCK_ALL             0x00
#define LOCK_TOPLEFT         ENABLE_RIGHTBOTTOM
#define LOCK_VERTICES        ENABLE_EDGES
#define LOCK_EDGES           ENABLE_VERTICES

#define DebugPrintf(str, ...) {\
	wchar_t buf[256] = L"\0";\
	swprintf_s(buf, 256, str, __VA_ARGS__);\
	OutputDebugString(buf); }
