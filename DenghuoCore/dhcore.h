// Denghuo
// Aurthor: Liang Tang
// Update: 08-04-14
#ifndef _DENGHUO_CORE_H_
#define _DENGHUO_CORE_H_
#include "dhcore.h"
#include "dherrorcode.h"
#include "dhconnection_win.h"
#include "dhfetcher.h"

typedef struct _dhCoreData
{
	// Connection Set
	dhConnectionSet* pConnSet;
	// Fetcher
	void* fetcherData;
	dhFetcher fetcher;

	// Paramaters
	int nMaxFetchCount;
	int nMaxConnectionSetSize;
	char szFetcherConf[512];
}dhCoreData;

int dhCore_start(void);

int dhCore_init(dhCoreData* coreData, const char* conffile);
int dhCore_run(dhCoreData* coreData);
int dhCore_close(dhCoreData* coreData);

#endif
