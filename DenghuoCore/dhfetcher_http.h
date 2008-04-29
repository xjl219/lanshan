// Denghuo
// Aurthor: Liang Tang
// Update: 08-04-14
#ifndef _DENGHUO_CORE_FETCHER_HTML_H_
#define _DENGHUO_CORE_FETCHER_HTML_H_
#include "dhfetcher.h"
#include "dhhashtable.h"
#include <stdio.h>

#define DHFETCHER_HTTP_MAX_INIT_URL 16
#define DHFETCHER_HTTP_HOST_MAXLEN 256
#define DHFETCHER_HTTP_URL_MAXLEN 512

typedef struct _dhFetcherData_Http
{
	char szInitURLs[DHFETCHER_HTTP_MAX_INIT_URL][DHFETCHER_HTTP_URL_MAXLEN];
	int  nNumInitURLs;
	int  nFileRecordNumber;
	dhHashtable fetchedURLHashtable;
	dhHashtable unfetchedURLHashtable;
	FILE* logfp;
}dhFetcherData_Http;

int dhFetcher_Http_SetFetcher(dhFetcher* fetcher);

// Interfaces
int dhFetcher_Http_CreateData(void** spfetcherdata, const char* conf);
int dhFetcher_Http_ReleaseData(void* fetcherdata);
int dhFetcher_Http_Request(dhFetcherRequest* request, void* fetcherdata);
int dhFetcher_Http_Response(char* response, int responselen, const char* identifier, void* fetcherdata);
// Tools
int dhFetcher_Http_ParseURL(const char* url,char* host, char* path);
int dhFetcher_Http_CompleteURL(const char* curUrl,char* addpath, char* completeUrl);
int dhFetcher_Http_SaveHtmlFile(const char* url, const char* html, dhFetcherData_Http* httpdata);


#endif
