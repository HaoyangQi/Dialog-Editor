#include "windowlist.h"

WINDOW_LIST* CreateWindowList()
{
	WINDOW_LIST* pList = (WINDOW_LIST*)malloc(sizeof(WINDOW_LIST));

	if (!pList) {
		return NULL;
	}

	memset(pList, 0, sizeof(WINDOW_LIST));
	return pList;
}

void ReleaseWindowList(WINDOW_LIST* pList)
{
	if (pList) {
		WINDOW_ITEM* pCurrent = pList->begin;
		WINDOW_ITEM* pTmp;

		while (pCurrent) {
			pTmp = pCurrent->next;
			free(pCurrent);
			pCurrent = pTmp;
		}

		free(pList);
	}
}

WINDOW_ITEM* AddWindow(WINDOW_LIST* pList, HWND hwnd, LONG handleProperty)
{
	WINDOW_ITEM* pwi = (WINDOW_ITEM*)malloc(sizeof(WINDOW_ITEM));

	if (!pwi) {
		return NULL;
	}

	memset(pwi, 0, sizeof(WINDOW_ITEM));
	pwi->hwnd = hwnd;
	pwi->handle_property = handleProperty;
	pwi->next = pList->begin;

	pList->begin = pwi;
	if (pList->end == NULL) {
		pList->end = pwi;
	}

	return pwi;
}

void DeleteWindow(WINDOW_LIST* pList, HWND hwnd)
{
	DeleteWindowItem(pList, WindowListFind(pList, hwnd));
}

void DeleteWindowItem(WINDOW_LIST* pList, WINDOW_ITEM* pwe)
{
	if (pwe) {
		if (pwe->prev) {
			pwe->prev->next = pwe->next;
		}
		else {
			pList->begin = pwe->next;
		}

		if (pwe->next) {
			pwe->next->prev = pwe->prev;
		}
		else {
			pList->end = pwe->prev;
		}

		DestroyWindow(pwe->hwnd);
		free(pwe);
	}
}

WINDOW_ITEM* WindowListFind(const WINDOW_LIST* pList, const HWND hwnd)
{
	WINDOW_ITEM* pwe = pList->begin;

	while (pwe) {
		if (pwe->hwnd == hwnd) {
			return pwe;
		}
		pwe = pwe->next;
	}

	return NULL;
}

// ----------------------------------------------------------------------------------
// Selection List



