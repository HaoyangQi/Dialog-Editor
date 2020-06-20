#include "ll.h"
#include "global.h"

void debugControlList(DESIGNER_CONTROL_LIST* list)
{
	DESIGNER_CONTROL_ITEM* p = list->begin;

	OutputDebugString(L"CONTROL LIST: \n    ");
	while (p) {
		DebugPrintf(L"%x -> ", (unsigned long long)(p->hwnd));
		p = p->next;
	}
	OutputDebugString(L"NULL\n");
}

void debugSelectionList(DESIGNER_SELECTION_LIST* list)
{
	DESIGNER_SELECTION_LIST* p = list;

	OutputDebugString(L"SELECTION LIST: \n    ");
	while (p) {
		DebugPrintf(L"%x -> ", (unsigned long long)(p->item->hwnd));
		p = p->next;
	}
	OutputDebugString(L"NULL\n");
}

DESIGNER_CONTROL_LIST* NewDesignerControlList()
{
	DESIGNER_CONTROL_LIST* pdcl = (DESIGNER_CONTROL_LIST*)malloc(sizeof(DESIGNER_CONTROL_LIST));
	memset(pdcl, 0, sizeof(DESIGNER_CONTROL_LIST));
	return pdcl;
}

void ReleaseDesignerControlList(DESIGNER_CONTROL_LIST* pdcl)
{
	DESIGNER_CONTROL_ITEM* pCurrent = pdcl->begin;
	DESIGNER_CONTROL_ITEM* pTmp;

	while (pCurrent) {
		pTmp = pCurrent->next;
		free(pCurrent);
		pCurrent = pTmp;
	}

	free(pdcl);
}

DESIGNER_CONTROL_ITEM* AddDesignerControlItem(DESIGNER_CONTROL_LIST* pdcl, HWND hwnd)
{
	DebugPrintf(L"[ADD] Adding handle: %x\n", (unsigned long long)hwnd);

	DESIGNER_CONTROL_ITEM* pdci = FindDesignerControlItem(pdcl, hwnd);

	if (pdci) {
		DebugPrintf(L"[ADD] Duplicate handle: %x\n", (unsigned long long)hwnd);
		return pdci;
	}

	pdci = (DESIGNER_CONTROL_ITEM*)malloc(sizeof(DESIGNER_CONTROL_ITEM));
	memset(pdci, 0, sizeof(DESIGNER_CONTROL_ITEM));

	pdci->hwnd = hwnd;
	pdci->prev = pdcl->end;
	pdcl->end = pdci;
	if (!pdcl->begin) {
		pdcl->begin = pdci;
	}

	return pdci;
}

DESIGNER_CONTROL_ITEM* FindDesignerControlItem(DESIGNER_CONTROL_LIST* pdcl, HWND hwnd)
{
	DESIGNER_CONTROL_ITEM* pdci = pdcl->begin;

	while (pdci) {
		if (pdci->hwnd == hwnd) {
			break;
		}
		pdci = pdci->next;
	}

	// if there is a hit, move up 1
	if (pdci && pdci != pdcl->begin) {
		DESIGNER_CONTROL_ITEM* swap = pdci->prev;

		if (swap->prev) {
			swap->prev->next = pdci;
		}
		if (pdci->next) {
			pdci->next->prev = swap;
		}
		pdci->prev = swap->prev;
		swap->prev = pdci;
		swap->next = pdci->next;
		pdci->next = swap;
	}

	return pdci;
}

// SELECTION LIST

DESIGNER_SELECTION_LIST* AccumulateSelection(DESIGNER_CONTROL_LIST* pdcl, DESIGNER_SELECTION_LIST* pdsl, HWND hwnd)
{
	if (!hwnd || FindFromSelection(pdsl, hwnd)) {
		return pdsl;
	}

	DESIGNER_SELECTION_LIST* sel = (DESIGNER_SELECTION_LIST*)malloc(sizeof(DESIGNER_SELECTION_LIST));
	//memset(sel, 0, sizeof(DESIGNER_SELECTION_LIST));

	sel->item = FindDesignerControlItem(pdcl, hwnd);
	sel->next = pdsl;

	if (!sel->item) {
		free(sel);
		return pdsl;
	}
	return sel;
}

DESIGNER_SELECTION_LIST* ResetDesignerSelectionList(DESIGNER_CONTROL_LIST* pdcl, DESIGNER_SELECTION_LIST* pdsl, HWND hwnd)
{
	if (pdsl && pdsl->item->hwnd == hwnd && !pdsl->next) {
		return pdsl;
	}

	ClearDesignerSelectionList(pdsl);
	return AccumulateSelection(pdcl, NULL, hwnd);
}

void ClearDesignerSelectionList(DESIGNER_SELECTION_LIST* pdsl)
{
	DESIGNER_SELECTION_LIST* tmp;

	while (pdsl) {
		tmp = pdsl->next;
		free(pdsl);
		pdsl = tmp;
	}
}

DESIGNER_CONTROL_ITEM* FindFromSelection(DESIGNER_SELECTION_LIST* pdsl, HWND hwnd)
{
	while (pdsl) {
		if (pdsl->item->hwnd == hwnd) {
			return pdsl->item;
		}
		pdsl = pdsl->next;
	}
	return NULL;
}
