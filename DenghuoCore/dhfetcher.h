// Denghuo
// Aurthor: Liang Tang
// Update: 08-04-14
#ifndef _DENGHUO_CORE_FETCHER_H_
#define _DENGHUO_CORE_FETCHER_H_

#define DHFETCHER_HOST_LEN 256
#define DHFETCHER_REQUEST_LEN 512
#define DFFETCHER_IDENTIFIER_LEN 512
typedef struct _dhFetcherRequest
{
	char szRequest[DHFETCHER_REQUEST_LEN];
	char szHost[DHFETCHER_HOST_LEN];
	int  nPort;
	char szIdentfier[DFFETCHER_IDENTIFIER_LEN];
}dhFetcherRequest;


typedef int (*dhFetcher_CreateData)(void** spfetcherdata,const char* conf);
typedef int (*dhFetcher_ReleaseData)(void* fetcherdata);
typedef int (*dhFetcher_Request)(dhFetcherRequest* request, void* fetcherdata);
typedef int (*dhFetcher_Response)(char* response, int responselen, const char* identifier, void* fetcherdata);

typedef struct _dhFetcher
{
	dhFetcher_CreateData createData;
	dhFetcher_ReleaseData releaseData;
	dhFetcher_Request request;
	dhFetcher_Response response;
	int nMaxPageSize;
}dhFetcher;


#endif
