#pragma once

#include <windows.h>

typedef struct _item {
	HWND hwnd;
	//LONG handle_property;
	struct _item* prev;
	struct _item* next;
} DESIGNER_CONTROL_ITEM;

typedef struct {
	//size_t count;
	DESIGNER_CONTROL_ITEM* begin;
	DESIGNER_CONTROL_ITEM* end;
} DESIGNER_CONTROL_LIST;

DESIGNER_CONTROL_LIST* NewDesignerControlList();
void ReleaseDesignerControlList(DESIGNER_CONTROL_LIST*);
DESIGNER_CONTROL_ITEM* AddDesignerControlItem(DESIGNER_CONTROL_LIST*, HWND);
DESIGNER_CONTROL_ITEM* FindDesignerControlItem(DESIGNER_CONTROL_LIST*, HWND);

// queue
typedef struct _selection {
	DESIGNER_CONTROL_ITEM* item;
	struct _selection* next;
} DESIGNER_SELECTION_LIST;

DESIGNER_SELECTION_LIST* AccumulateSelection(DESIGNER_CONTROL_LIST*, DESIGNER_SELECTION_LIST*, HWND);
DESIGNER_SELECTION_LIST* ResetDesignerSelectionList(DESIGNER_CONTROL_LIST*, DESIGNER_SELECTION_LIST*, HWND);
void ClearDesignerSelectionList(DESIGNER_SELECTION_LIST*);
DESIGNER_CONTROL_ITEM* FindFromSelection(DESIGNER_SELECTION_LIST*, HWND);

void debugControlList(DESIGNER_CONTROL_LIST*);
void debugSelectionList(DESIGNER_SELECTION_LIST*);
