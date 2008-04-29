// Denghuo
// Aurthor: Liang Tang
// Update: 08-04-14
#include "dhfetcher_http.h"
#include "dherrorcode.h"
#include "dhtools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char DHFETCHER_HTTP_GETREQUEST[] = "GET %s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nUser-agent:Mozilla/4.0\r\nAccept:*/*\r\n\n";
static const int  DHFETCHER_HTTP_MAXPAGESIZE  = 500*1024;
static const int  DHFETCHER_HTTP_URL_HASHSIZE = 99;
static const int  DHFETCHER_HTTP_PORT = 80;
static const char DHFETCHER_HTTP_LOGFILE[] = "http_fetcher_log.txt";

int dhFetcher_Http_SetFetcher(dhFetcher* fetcher)
{
	fetcher->createData = dhFetcher_Http_CreateData;
	fetcher->releaseData = dhFetcher_Http_ReleaseData;
	fetcher->request = dhFetcher_Http_Request;
	fetcher->response = dhFetcher_Http_Response;
	fetcher->nMaxPageSize = DHFETCHER_HTTP_MAXPAGESIZE;
	return 0;
}

int dhFetcher_Http_CreateData(void** spfetcherdata,const char* conf)
{
	FILE* fp;
	dhFetcherData_Http* httpdata;
	int i;

	// memory allocation
	*spfetcherdata = (void*)malloc(sizeof(dhFetcherData_Http));
	httpdata = (dhFetcherData_Http*)(*spfetcherdata);

	// read the initial host and path from configuration file
	fp = fopen(conf, "r");
	if (fp == NULL)
		return DHERR_FILE_CANNOT_OPEN;
	i=0;
	while(fscanf(fp, "%s\n", httpdata->szInitURLs[i]) == 1)
	{
		i++;
		if (i >= DHFETCHER_HTTP_MAX_INIT_URL)
			break;
	}
	httpdata->nNumInitURLs = i;
	fclose(fp);

	// opent the fetcher log file
	httpdata->logfp = fopen(DHFETCHER_HTTP_LOGFILE, "w");
	if (httpdata->logfp == NULL)
	{
		printf("Cannot open the log file : %s\n", DHFETCHER_HTTP_LOGFILE);
		return DHERR_FILE_CANNOT_OPEN;
	}

	// create the url hashtable
	dhHashtable_Create(&httpdata->fetchedURLHashtable,   DHFETCHER_HTTP_URL_HASHSIZE);
	dhHashtable_Create(&httpdata->unfetchedURLHashtable, DHFETCHER_HTTP_URL_HASHSIZE);

	// insert the initial website into the unfetched url hashtable
	for (i=0; i<httpdata->nNumInitURLs; i++)
	{
		dhHashtable_Insert(&httpdata->unfetchedURLHashtable, httpdata->szInitURLs[i], NULL);
	}

	// zero the file record number
	httpdata->nFileRecordNumber = 0;

	return 0;
}

int dhFetcher_Http_ReleaseData(void* fetcherdata)
{
	dhFetcherData_Http* httpdata =(dhFetcherData_Http*)fetcherdata;
	// close the log file
	fclose(httpdata->logfp);
	// release the url hashtable
	dhHashtable_Release(&httpdata->fetchedURLHashtable);
	dhHashtable_Release(&httpdata->unfetchedURLHashtable);
	return 0;
}

int dhFetcher_Http_Request(dhFetcherRequest* request, void* fetcherdata)
{
	int nRet;
	char url[DHFETCHER_HTTP_URL_MAXLEN];
	char host[DHFETCHER_HTTP_HOST_MAXLEN];
	char path[DHFETCHER_HTTP_URL_MAXLEN];
	dhFetcherData_Http* httpdata = (dhFetcherData_Http*)fetcherdata;
	
	if (dhHashtable_Count(&httpdata->unfetchedURLHashtable) == 0)
		return -1;

	// Get the next unfetched URL
	nRet = dhHashtable_GetFirst(&httpdata->unfetchedURLHashtable, url, NULL);
	DH_ON_FAILED_RETURN(nRet);

	// Remove from unfetched URL hashtable and Insert to fetched URL hashtable
	nRet = dhHashtable_RemoveFirst(&httpdata->unfetchedURLHashtable);
	DH_ON_FAILED_RETURN(nRet);
	
	nRet = dhHashtable_Insert(&httpdata->fetchedURLHashtable, url, NULL);
	DH_ON_FAILED_RETURN(nRet);

	// Parse the URL
	nRet = dhFetcher_Http_ParseURL(url, host, path);
	DH_ON_FAILED_RETURN(nRet);
	
	// Construct the get request
	sprintf(request->szRequest, DHFETCHER_HTTP_GETREQUEST, path, host);
	strcpy(request->szHost, host);
	strcpy(request->szIdentfier, url);
	request->nPort = DHFETCHER_HTTP_PORT;

	return 0;
}

int dhFetcher_Http_Response(char* response, int responselen, const char* identifier, void* fetcherdata)
{
	int nRet;
	const char* buf_html_start;
	const char* buf_next_ahref;
	const char* buf_href_link;
	dhFetcherData_Http* httpdata = (dhFetcherData_Http*)fetcherdata;
	const char* curUrl = identifier;

	// const char* buf_mark;
	char hyperlink[DHFETCHER_HTTP_URL_MAXLEN];
	char host[DHFETCHER_HTTP_HOST_MAXLEN];
	char path[DHFETCHER_HTTP_URL_MAXLEN];
	char dirUrl[DHFETCHER_HTTP_URL_MAXLEN];
	//char mark;
	int  count;

	// printf("%s\n", response);
	// locate html start position
	buf_html_start = strstr(response, "<html");
	if (buf_html_start == NULL)
		return 0;

	// retrieve the hyperlinks and construct new requests
	buf_next_ahref = buf_html_start;
	while((buf_next_ahref = strstr(buf_next_ahref, "<a href=\"")) != NULL)
	{
		// jump to the hyperlink
		buf_next_ahref += sizeof("<a href=\"")-1;
		if (*buf_next_ahref == '\0')
			break;
		buf_href_link = buf_next_ahref;

		// retrieve the link url
		buf_next_ahref = strchr(buf_next_ahref, '"');
		if (buf_next_ahref == NULL)
			break;

		count = buf_next_ahref-buf_href_link;
		strncpy(hyperlink, buf_href_link, count);
		hyperlink[count] = '\0';

		// Add the hyperlink into unfetched URL hashtable
		printf("link : %s\n", hyperlink);
		fprintf(httpdata->logfp, "link : %s\n", hyperlink);

		// Verify this hyperlink is a normally url
		if (DH_IF_SUCCESS(dhFetcher_Http_ParseURL(hyperlink, host, path)))
		{
			if (host[0] == '\0')
			{
				// Convert to complete URL
				nRet = dhFetcher_Http_CompleteURL(curUrl, path, hyperlink);
				if (DH_IF_FAILED(nRet))
					goto _OutAndContinue;
				printf("converted link : %s\n", hyperlink);
				fprintf(httpdata->logfp, "convertedlink : %s\n", hyperlink);
			}

			if (!dhHashtable_Find(&httpdata->fetchedURLHashtable, hyperlink, NULL))
			{
				dhHashtable_Insert(&httpdata->unfetchedURLHashtable, hyperlink, NULL);
			}
		}

_OutAndContinue:

		// jump out the href tag
		buf_next_ahref +=2;
	}

	// Save the html to disk file
	nRet = dhFetcher_Http_SaveHtmlFile(curUrl, buf_html_start, httpdata);
	DH_ON_FAILED_RETURN(nRet);

	return 0;
}

int dhFetcher_Http_ParseURL(const char* url,char* host, char* path)
{
	url += strspn(url, " \t"); // skip blank characters
	
	if (*url == '\0'|| *url == '#' || *url == '.')
		return -1;

	if (*url == '/') // No host name
	{
		if (host)
			host[0] = 0;
		if (path) 
			strcpy(path, url);
		return 0;
	}
	else if (_strnicmp(url, "http://", 7) == 0) // contain "http://" which consists of 7 chars
	{
		// contain host name
		url += 7;
		if (*url == '\0') // No path name
		{
			if (host) 
				strcpy(host, url);
			if (path) 
				strcpy(path, "/");
		}
		else
		{
			int count;
			const char* pathstart = strchr(url, '/');
			if (pathstart == NULL)
			{
				if (host) strcpy(host, url);
				if (path) strcpy(path, "/");
			}
			else
			{
				count = pathstart-url;
				if (host)
				{
					strncpy(host, url, pathstart-url);
					host[count] = '\0';
				}
				if (path)
					strcpy(path, pathstart);
			}
		}
	}
	else // additional path
	{
		if (strchr(url, ':'))
			return -1;
		if (host)
			host[0] = 0;
		if (path)
			strcpy(path, url);
	}
	return 0;
}

int dhFetcher_Http_CompleteURL(const char* curUrl, char* addpath, char* completeUrl)
{
	int nRet;
	char host[DHFETCHER_HTTP_HOST_MAXLEN];

	// Convert to complete URL
	nRet = dhFetcher_Http_ParseURL(curUrl, host, NULL);
	if (DH_IF_FAILED(nRet))
		return -1;
	if (addpath[0] == '/') // Additional path from root
		sprintf(completeUrl, "http://%s%s",host, addpath);
	else // Additional path from current directory
	{
		int pathlen = strlen(addpath);
		int count;
		const char* dirp = strrchr(curUrl, '/');
		if (dirp == NULL)
			return -1;
		count = dirp - curUrl + 1;
		if (count == sizeof("http://")-1) // if curUrl is a complete domain name, eg: http://www.xxx.com,
		{
			sprintf(completeUrl, "http://%s/%s",host, addpath);
		}
		else
		{
			strncpy(completeUrl, curUrl, count);
			strncpy(completeUrl+count, addpath, pathlen);
			completeUrl[count+pathlen] = '\0';
		}
	}
	return 0;
}

int dhFetcher_Http_SaveHtmlFile(const char* url, const char* html, dhFetcherData_Http* httpdata)
{
	FILE* fp;
	char szFilename[64];
	sprintf(szFilename, "page%d.txt", httpdata->nFileRecordNumber);
	fp = fopen(szFilename, "w");
	if (fp == NULL)
		return -1;
	fprintf(fp, "%s\n", url);
	fprintf(fp, "%s\n", html);
	fclose(fp);
	httpdata->nFileRecordNumber++;
	return 0;
}
