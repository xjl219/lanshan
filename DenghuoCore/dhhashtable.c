// Denghuo
// Aurthor: Liang Tang
// Update: 08-04-16
#include "dhhashtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int dhHashtable_Hashcode(dhHashtable* hashtable, const char* key) 
{ 
	int ch = 0; 
	while(*key) ch += *key++; 
	return ch % hashtable->nHashsize;
}

int dhHashtable_Create(dhHashtable* hashtable, int hashsize)
{
	int i;
	hashtable->nHashsize = hashsize;
	hashtable->nCount = 0;
	hashtable->pItems = (dhHashtableItem*)malloc(sizeof(dhHashtableItem) * hashsize);
	hashtable->spLastItems = (dhHashtableItem**)malloc(sizeof(dhHashtableItem*) * hashsize);
	for (i=0; i<hashsize; i++)
	{
		hashtable->pItems[i].key = NULL;
		hashtable->pItems[i].pNext = NULL;
		hashtable->pItems[i].value = NULL;
		hashtable->spLastItems[i] = &hashtable->pItems[i];
	}
	return 0;
}

int dhHashtable_Release(dhHashtable* hashtable)
{
	int i;
	dhHashtableItem* pItem;
	dhHashtableItem* pNext;
	for (i=0; i<hashtable->nHashsize; i++)
	{
		pItem = hashtable->pItems[i].pNext;
		while(pItem)
		{
			pNext = pItem->pNext;
			free(pItem->key);
			free(pItem);
			pItem = pNext;
		}
		hashtable->pItems[i].pNext = NULL;
		hashtable->spLastItems[i] = NULL;
	}
	hashtable->nHashsize = 0;
	hashtable->nCount = 0;
	return 0;
}

int dhHashtable_Insert(dhHashtable* hashtable, const char* key, void* value)
{
	int hashcode;
	dhHashtableItem *pNewItem;

	hashcode = dhHashtable_Hashcode(hashtable, key);
	pNewItem = (dhHashtableItem*)malloc(sizeof(dhHashtableItem));
	pNewItem->key = _strdup(key);
	pNewItem->value = value;
	pNewItem->pNext = NULL;
	hashtable->spLastItems[hashcode]->pNext = pNewItem;
	hashtable->spLastItems[hashcode] = pNewItem;
	hashtable->nCount++;
	return 0;
}

int dhHashtable_Count(dhHashtable* hashtable)
{
	return hashtable->nCount;
}

int dhHashtable_Find(dhHashtable* hashtable, const char* key, void** spvalue)
{
	int hashcode;
	dhHashtableItem *pItem;

	hashcode = dhHashtable_Hashcode(hashtable, key);
	pItem = hashtable->pItems[hashcode].pNext;
	while(pItem)
	{
		if (strcmp(pItem->key, key) == 0)
		{
			if (spvalue != NULL)
				*spvalue = pItem->value;
			return 1;
		}
		else
		{
			pItem = pItem->pNext;
		}
	}
	return 0;
}

int dhHashtable_GetFirst(dhHashtable* hashtable, const char* key, void** spvalue)
{
	int nItemIndex;
	dhHashtableItem* pItem;
	for (nItemIndex=0; nItemIndex <hashtable->nHashsize; nItemIndex++)
	{
		pItem = hashtable->pItems[nItemIndex].pNext;
		if (pItem != NULL)
		{
			strcpy(key, pItem->key);
			if (spvalue != NULL)
				*spvalue = pItem->value;
			return 0;
		}
	}
	return -1;
}

int dhHashtable_RemoveFirst(dhHashtable* hashtable)
{
	int nItemIndex;
	dhHashtableItem* pItem;
	dhHashtableItem* pLastItem;
	for (nItemIndex=0; nItemIndex <hashtable->nHashsize; nItemIndex++)
	{
		pLastItem = &hashtable->pItems[nItemIndex];
		pItem = hashtable->pItems[nItemIndex].pNext;
		if (pItem != NULL)
		{
			pLastItem->pNext = pItem->pNext;
			if (hashtable->spLastItems[nItemIndex] == pItem)
				hashtable->spLastItems[nItemIndex] = &hashtable->pItems[nItemIndex];

			// free the item
			free(pItem->key);
			free(pItem);

			hashtable->nCount--;
			return 0;
		}
	}
	return -1;
}
