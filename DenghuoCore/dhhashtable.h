// Denghuo
// Aurthor: Liang Tang
// Update: 08-04-16
#ifndef _DENGHUO_CORE_HASHTABLE_H_
#define _DENGHUO_CORE_HASHTABLE_H_

typedef struct _dhHashtableItem
{
	char* key;
	void* value;
	struct _dhHashtableItem* pNext;
}dhHashtableItem;

typedef struct _dhHashtable
{
	dhHashtableItem *pItems;
	dhHashtableItem **spLastItems;
	int nHashsize;
	int nCount;
}dhHashtable;

typedef struct _dhHashtableIterator
{
	int nItemIndex;
	dhHashtableItem* pCurrentItem;
}dhHashtableIterator;

int dhHashtable_Create(dhHashtable* hashtable, int hashsize);
int dhHashtable_Release(dhHashtable* hashtable);
int dhHashtable_Insert(dhHashtable* hashtable, const char* key, void* value);
int dhHashtable_Count(dhHashtable* hashtable);
int dhHashtable_Find(dhHashtable* hashtable, const char* key, void** spvalue);
int dhHashtable_GetFirst(dhHashtable* hashtable, const char* key, void** spvalue);
int dhHashtable_RemoveFirst(dhHashtable* hashtable);


#endif
