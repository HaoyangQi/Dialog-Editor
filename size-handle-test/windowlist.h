#pragma once

#include "framework.h"

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

typedef struct _window {
	HWND hwnd;
	RECT rect;
	LONG disable;

	struct _window* prev;
	struct _window* next;
} WINDOW_GEOMETRY;

typedef struct {
	BOOL bVisible;

	WINDOW_GEOMETRY* base;
	WINDOW_GEOMETRY* focus;
	WINDOW_GEOMETRY* begin;
	WINDOW_GEOMETRY* end;
} WINDOW_LIST;

WINDOW_LIST* NewWindowList(HWND, BOOL);
void ReleaseWindowList(WINDOW_LIST*);
WINDOW_GEOMETRY* AddWindow(WINDOW_LIST*, HWND, LONG);
void DeleteWindow(WINDOW_LIST*, HWND);
void DeleteWindowByReference(WINDOW_LIST*, WINDOW_GEOMETRY*);
WINDOW_GEOMETRY* WindowListFind(const WINDOW_LIST*, const HWND);
WINDOW_GEOMETRY* WindowListHitTest(const WINDOW_LIST*, const POINT);
void WindowListUpdateWindowPosition(WINDOW_GEOMETRY*, int, int);
