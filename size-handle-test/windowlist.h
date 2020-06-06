#pragma once

#include "framework.h"

#define POS_TOP_LEFT_HANDLE      0x01
#define POS_TOP_CENTER_HANDLE    0x02
#define POS_TOP_RIGHT_HANDLE     0x04
#define POS_MIDDLE_LEFT_HANDLE   0x08
#define POS_MIDDLE_RIGHT_HANDLE  0x10
#define POS_BOTTOM_LEFT_HANDLE   0x20
#define POS_BOTTOM_CENTER_HANDLE 0x40
#define POS_BOTTOM_RIGHT_HANDLE  0x80

#define LOCK_NONE                0x00
#define LOCK_ALL                 0xFF
#define LOCK_VERTICES            0xA5
#define LOCK_EDGES               0x5A
#define LOCK_TOP_LEFT            0x2F

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
