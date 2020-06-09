#pragma once

#include "framework.h"

typedef struct _item {
	HWND hwnd;
	LONG handle_property;
	struct _item* prev;
	struct _item* next;
} WINDOW_ITEM;

typedef struct {
	WINDOW_ITEM* begin;
	WINDOW_ITEM* end;
} WINDOW_LIST;

typedef struct _selection {
	WINDOW_ITEM* pwi;
	struct _selection* prev;
	struct _selection* next;
} SELECTION_ITEM;

typedef struct {
	SELECTION_ITEM* begin;
	SELECTION_ITEM* end;
} SELECTION_LIST;

// window list keeps data, deep-free
WINDOW_LIST* CreateWindowList();
void ReleaseWindowList(WINDOW_LIST*);
WINDOW_ITEM* AddWindow(WINDOW_LIST*, HWND, LONG);
void DeleteWindow(WINDOW_LIST*, HWND); // a wrapper
void DeleteWindowItem(WINDOW_LIST*, WINDOW_ITEM*); // DestroyWindow will be called
WINDOW_ITEM* WindowListFind(const WINDOW_LIST*, const HWND);

// selection list is shallow-copy, it does not free item, except DeleteSelectXXX
SELECTION_LIST* CreateSelectionList();
void ReleaseSelectionList(SELECTION_LIST*);
void AddSelection(SELECTION_LIST*, WINDOW_ITEM*);
SELECTION_ITEM* FindSelectedWindow(const SELECTION_LIST*, const HWND);
void PopSelectedWindow(SELECTION_LIST*, const HWND);
void PopSelectionItem(SELECTION_LIST*, WINDOW_ITEM*);
void ClearSelection(SELECTION_LIST*);
// deep-free wrapper functions, deletion work will be forwarded to WINDOW_LIST APIs
void DeleteSelectedWindow(WINDOW_LIST*, SELECTION_LIST*, HWND); // DeleteWindow will be called
void DeleteSelection(WINDOW_LIST*, SELECTION_LIST*); // DeleteWindowItem will be called

