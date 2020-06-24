#include "dlgdata.h"

void TemplateEnqueControlItem(DIALOG_TEMPLATE_DATA* pdtd, DIALOG_TEMPLATE_ITEM_DATA* pItem)
{
	pItem->prev = NULL;
	if (pdtd->pItemBegin == NULL) {
		pItem->next = NULL;
		pdtd->pItemBegin = pItem;
		pdtd->pItemEnd = pItem;
	}
	else {
		pItem->next = pdtd->pItemBegin;
		pdtd->pItemBegin = pItem;
	}
}

DIALOG_TEMPLATE_ITEM_DATA* TemplateFindByItemID(DIALOG_TEMPLATE_DATA* pdtd, int id)
{
	DIALOG_TEMPLATE_ITEM_DATA* pItem = pdtd->pItemBegin;

	while (pItem) {
		if (pItem->item_id == id) {
			return pItem;
		}
		pItem = pItem->next;
	}

	return NULL;
}

DIALOG_TEMPLATE_ITEM_DATA* TemplateFindByControlID(DIALOG_TEMPLATE_DATA* pdtd, int id)
{
	DIALOG_TEMPLATE_ITEM_DATA* pItem = pdtd->pItemBegin;

	while (pItem) {
		if (pItem->id == id) {
			return pItem;
		}
		pItem = pItem->next;
	}

	return NULL;
}

void TemplateDeleteByReference(DIALOG_TEMPLATE_DATA* pdtd, DIALOG_TEMPLATE_ITEM_DATA* pItem)
{
	if (pItem->prev) {
		pItem->prev->next = pItem->next;
	}
	else {
		pdtd->pItemBegin = pItem->next;
	}

	if (pItem->next) {
		pItem->next->prev = pItem->prev;
	}
	else {
		pdtd->pItemEnd = pItem->prev;
	}

	free(pItem->windowClass);
	free(pItem->title);
	free(pItem->extraData);
	free(pItem);

	pdtd->cDlgItems--;
}

void TemplateDeleteByItemID(DIALOG_TEMPLATE_DATA* pdtd, int id)
{
	TemplateDeleteByReference(pdtd, TemplateFindByControlID(pdtd, id));
}

void TemplateDeleteByControlID(DIALOG_TEMPLATE_DATA* pdtd, int id)
{
	TemplateDeleteByReference(pdtd, TemplateFindByControlID(pdtd, id));
}

DIALOG_TEMPLATE_ITEM_DATA* TemplateAddControl(DIALOG_TEMPLATE_DATA* pdtd)
{
	DIALOG_TEMPLATE_ITEM_DATA* pItem = (DIALOG_TEMPLATE_ITEM_DATA*)malloc(sizeof(DIALOG_TEMPLATE_ITEM_DATA));

	memset(pItem, 0, sizeof(DIALOG_TEMPLATE_ITEM_DATA));
	pItem->item_id = pdtd->item_id_count++;
	TemplateEnqueControlItem(pdtd, pItem);
	pdtd->cDlgItems++;

	return pItem;
}
