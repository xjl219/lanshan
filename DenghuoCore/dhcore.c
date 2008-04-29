// Denghuo
// Aurthor: Liang Tang
// Update: 08-04-14
#include <stdio.h>
#include "dhcore.h"
#include "dherrorcode.h"
#include "dhconnection_win.h"
#include "dhfetcher.h"
#include "dhfetcher_http.h"


static const char szConfFilename[] = "conf.txt";
static const int MAX_FETCH_COUNT = 1;
static const int MAX_CONNECTION = 1;
static const char DEFAULT_FETCHER_CONF[] ="http_conf.txt";

int dhCore_start(void)
{
	int nRet;
	dhCoreData coreData;
	
	dhConn_startup();

	nRet = dhCore_init(&coreData, szConfFilename);
	DH_ON_FAILED_RETURN(nRet);

	nRet = dhCore_run(&coreData);
	DH_ON_FAILED_RETURN(nRet);

	nRet = dhCore_close(&coreData);
	DH_ON_FAILED_RETURN(nRet);

	dhConn_cleanup();
	system("pause");
	return 0;
}

int dhCore_init(dhCoreData* coreData, const char* conffile)
{
	char szParamName[128];
	char szParamVal[128];
	FILE *fp;

	// Initialize the default parameters
	coreData->nMaxConnectionSetSize = MAX_CONNECTION;
	coreData->nMaxFetchCount = MAX_FETCH_COUNT;
	strcpy(coreData->szFetcherConf, DEFAULT_FETCHER_CONF);

	// Open the configuration file and read the parameters
	fp = fopen(conffile, "r");
	if (fp == NULL)
	{
		printf("Cannot open the configuration file : %s\n", conffile);
	}
	else
	{
		while(fscanf(fp, "%s %s\n",szParamName, szParamVal) == 2)
		{
			if (_stricmp(szParamName, "fetchcount") == 0)
				coreData->nMaxFetchCount = atoi(szParamVal);
			else if (_stricmp(szParamName, "connectionsize") == 0)
				coreData->nMaxConnectionSetSize = atoi(szParamVal);
			else if (_stricmp(szParamName, "fetcherconf") == 0)
				strcpy(coreData->szFetcherConf, szParamVal);
			else
			{
				printf("Unknown parameter : %s in configruation file\n", szParamName);
			}
		}
		fclose(fp);
	}

	// Set http fetcher
	dhFetcher_Http_SetFetcher(&coreData->fetcher);
	// Create the fetcher data
	coreData->fetcher.createData(&coreData->fetcherData, coreData->szFetcherConf);
	// Create connection
	dhConn_create(&coreData->pConnSet, coreData->nMaxConnectionSetSize);

	return 0;
}

int dhCore_run(dhCoreData* coreData)
{
	int i;
	int nRet;
	int nNumConn;
	int nFetchCount = 0;
	char** responseBuffers;
	int* responseBufferPoses;
	dhFetcherRequest request;
	char** requestIds;
	char szConn[DHFETCHER_REQUEST_LEN];

	// Create fetch buffers
	nNumConn = coreData->pConnSet->nMaxConn;
	responseBuffers = (char**)malloc(sizeof(char*)*nNumConn);
	responseBufferPoses = (int*)malloc(sizeof(int)*nNumConn);
	requestIds = (char**)malloc(sizeof(char*)*nNumConn);
	for (i=0; i<nNumConn; i++)
	{
		responseBuffers[i] = (char*)malloc(coreData->fetcher.nMaxPageSize);
		responseBufferPoses[i] = 0;
		requestIds[i] = (char*)malloc(sizeof(char)*DFFETCHER_IDENTIFIER_LEN);
	}

	while(1)
	{
		if (nFetchCount >= coreData->nMaxFetchCount)
			break;

		// Send requests
		for (i=0; i<nNumConn; i++)
		{
			if (dhConn_canconnect(coreData->pConnSet, i))
			{
				// Pick a request
				nRet = coreData->fetcher.request(&request, coreData->fetcherData);
				if (DH_IF_FAILED(nRet))
					break;

				// Construct the conection
				sprintf(szConn, "%s %d", request.szHost, request.nPort); 
				if (DH_IF_SUCCESS(dhConn_connect(coreData->pConnSet, szConn)))
				{
					// Send the request
					nRet = dhConn_write(coreData->pConnSet, i, request.szRequest, strlen(request.szRequest));
					printf("write %d bytes\n", nRet);
					if (nRet == -1)
					{
						printf("write SOCKET_ERROR!\n");
						dhConn_disconnect(coreData->pConnSet, i);
						responseBufferPoses[i] = 0;
						continue;
					}
					else if (nRet != strlen(request.szRequest))
					{
						printf("write bytes < request length!!!\n");
						while(1) {};
					}
					responseBufferPoses[i] = 0;
					// Save the request identifier with the connection index
					strcpy(requestIds[i], request.szIdentfier);
				}
			}
		}

		// Check all connection are not used
		if (dhConn_usedcount(coreData->pConnSet) == 0)
			break;

		// Wait responses
		dhConn_wait(coreData->pConnSet);

		// Read responses
		for (i=0; i<nNumConn; i++)
		{
			if (dhConn_canread(coreData->pConnSet, i))
			{
				int nPos = responseBufferPoses[i];
				int nAvaiSize = coreData->fetcher.nMaxPageSize - nPos;
				nRet = dhConn_read(coreData->pConnSet, i, responseBuffers[i]+nPos, nAvaiSize);
				if (nRet < 0)
				{
					// Disconnect the conection
					dhConn_disconnect(coreData->pConnSet, i);
					responseBufferPoses[i] = 0;

					printf("%d SOCKET_ERROR!\n", i);
				}
				else
				{
					responseBufferPoses[i] += nRet;
					printf("%d Recived %d bytes\n", i, nRet);

					if (responseBufferPoses[i] >= coreData->fetcher.nMaxPageSize || nRet == 0 ) // Finished reading response
					{
						responseBuffers[i][responseBufferPoses[i]] = '\0';
						// Parse the response buffer
						nRet = coreData->fetcher.response(responseBuffers[i],responseBufferPoses[i], requestIds[i], coreData->fetcherData);
						if (nRet < 0)
						{
							printf("%d Parse error!\n", i);
						}

						// Disconnect the conection
						dhConn_disconnect(coreData->pConnSet, i);
						responseBufferPoses[i] = 0;

						nFetchCount++;
					}
				}
			}
		}
	}

	// Free fetch buffers
	for (i=0; i<nNumConn; i++)
	{
		free(responseBuffers[i]);
		free(requestIds[i]);
	}
	free(responseBuffers);
	free(responseBufferPoses);
	free(requestIds);

	return 0;
}

int dhCore_close(dhCoreData* coreData)
{
	// Relase requests and fetcher
	coreData->fetcher.releaseData(coreData->fetcherData);

	// Close connection
	dhConn_disconnectAll(coreData->pConnSet);
	dhConn_release(coreData->pConnSet);

	return 0;
}


