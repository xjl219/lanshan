// Denghuo
// Aurthor: Liang Tang
// Update: 08-04-14

#include "dhconnection_win.h"
#include "dherrorcode.h"
#include "dhtools.h"
#include <stdio.h>
#include <string.h>

int dhConn_startup(void)
{
	WSADATA wsaData;
	int nRet =WSAStartup(MAKEWORD(2,2), &wsaData);
	if (nRet != 0)
		return -1;
	else
		return 0;
}

int dhConn_cleanup(void)
{
	return WSACleanup();
}

int dhConn_create(dhConnectionSet** ppConnSet, int nMaxConn)
{
	int i;
	*ppConnSet = (dhConnectionSet*)malloc(sizeof(dhConnectionSet));
	if (*ppConnSet == NULL)
		return -1;
	(*ppConnSet)->nMaxConn = nMaxConn;
	(*ppConnSet)->nUsedConnCount = 0;
	(*ppConnSet)->pSockets = (SOCKET*)malloc(sizeof(SOCKET)*nMaxConn);
	(*ppConnSet)->pServerAddrs = (SOCKADDR_IN*)malloc(sizeof(SOCKADDR_IN)*nMaxConn);
	(*ppConnSet)->pStates = (int*)malloc(sizeof(int)*nMaxConn);
	for (i=0; i<nMaxConn; i++)
	{
		(*ppConnSet)->pStates[i] = DHCONN_STATE_NOTCONNECTED;
	}
	return 0;
}

int dhConn_release(dhConnectionSet* pConnSet)
{
	if (pConnSet == NULL)
		return -1;
	else
	{
		free(pConnSet->pSockets);
		free(pConnSet->pServerAddrs);
		free(pConnSet->pStates);
		free(pConnSet);
		return 0;
	}
}

int dhConn_connect(dhConnectionSet* pConnSet, const char* pszConn)
{
	char szAddr[256];
	int  nPort;
	int nIndex;

	// Find not connected socket index
	for (nIndex=0;nIndex<pConnSet->nMaxConn; nIndex++)
	{
		if (pConnSet->pStates[nIndex] == DHCONN_STATE_NOTCONNECTED)
			break;
	}
	if (nIndex == pConnSet->nMaxConn) // Not available socket right now
		return DHERR_SOCKET_NOT_AVAILABLE;

	// Parse the connection address and port from the string parameter
	if (sscanf(pszConn,"%s %d",szAddr, &nPort) != 2)
		return DHERR_PARAMETER_FORMAT;

	// Create the socket
	pConnSet->pSockets[nIndex] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Fill with the connection address
	pConnSet->pServerAddrs[nIndex].sin_family = AF_INET;
    pConnSet->pServerAddrs[nIndex].sin_port = htons(nPort);    
	if (isalpha(szAddr[0]))
	{
		struct hostent *remoteHost;
		remoteHost = gethostbyname(szAddr);
		if (remoteHost == NULL) // Cannot find the host address by host name
			return DHERR_SOCKET_CANNOT_GET_HOSTNAME;
		pConnSet->pServerAddrs[nIndex].sin_addr.s_addr = *(u_long *) remoteHost->h_addr_list[0];
	}
	else
	{
		pConnSet->pServerAddrs[nIndex].sin_addr.s_addr = inet_addr(szAddr);
	}
	pConnSet->pStates[nIndex] = DHCONN_STATE_WAIT;

	if (connect(pConnSet->pSockets[nIndex], (SOCKADDR *)&pConnSet->pServerAddrs[nIndex], sizeof(SOCKADDR_IN)) != 0)
		return -1;

	pConnSet->nUsedConnCount++;
	return 0;
}

int dhConn_disconnect(dhConnectionSet* pConnSet, int nIndex)
{
	if (pConnSet->pStates[nIndex] == DHCONN_STATE_NOTCONNECTED)
		return 0;
	else
	{
		closesocket(pConnSet->pSockets[nIndex]);
		pConnSet->pStates[nIndex] = DHCONN_STATE_NOTCONNECTED;
		pConnSet->nUsedConnCount--;
		return 0;
	}
}

int dhConn_disconnectAll(dhConnectionSet* pConnSet)
{
	int i;
	for (i=0; i<pConnSet->nMaxConn; i++)
	{
		if (!dhConn_canconnect(pConnSet, i))
		{
			dhConn_disconnect(pConnSet, i);
		}
	}
	return 0;
}

int dhConn_wait(dhConnectionSet* pConnSet)
{
	fd_set readfds;
	int i;
	int nRet;

	// Clear the fds
	FD_ZERO(&readfds);
	/*FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);*/
	
	// Set the socket fd
	for (i=0; i<pConnSet->nMaxConn; i++)
	{
		if (pConnSet->pStates[i] & DHCONN_STATE_WAIT)
		{
			FD_SET(pConnSet->pSockets[i], &readfds);
		}
	}

	// Select
	nRet = select(0, &readfds, NULL, NULL, NULL);
	if (nRet == SOCKET_ERROR)
	{
		WSACleanup();
		return DHERR_SOCKET_ERROR;
	}
	else if (nRet == 0)
	{
		// time limit expired
		return 0;
	}
	else
	{
		// pick read or write states
		for (i=0; i<pConnSet->nMaxConn; i++)
		{
			if (pConnSet->pStates[i] & DHCONN_STATE_WAIT)
			{
				pConnSet->pStates[i] = DHCONN_STATE_WAIT;

				if (FD_ISSET(pConnSet->pSockets[i], &readfds))
					pConnSet->pStates[i] |= DHCONN_STATE_READY_READ;
			}
		}
	}


	return 0;
}

int dhConn_usedcount(dhConnectionSet* pConnSet)
{
	return pConnSet->nUsedConnCount;
}

int dhConn_canconnect(dhConnectionSet* pConnSet, int nIndex)
{
	return pConnSet->pStates[nIndex] == DHCONN_STATE_NOTCONNECTED;
}

int dhConn_canwrite(dhConnectionSet* pConnSet, int nIndex)
{
	return pConnSet->pStates[nIndex] & DHCONN_STATE_READY_WRITE;
}

int dhConn_canread(dhConnectionSet* pConnSet, int nIndex)
{
	return pConnSet->pStates[nIndex] & DHCONN_STATE_READY_READ;
}

int dhConn_write(dhConnectionSet* pConnSet, int nIndex, const char* pBuf, int nBufSize)
{
	return send(pConnSet->pSockets[nIndex], pBuf, nBufSize, 0);
}

int dhConn_read(dhConnectionSet* pConnSet, int nIndex, char* pBuf, int nBufSize)
{
	return recv(pConnSet->pSockets[nIndex], pBuf, nBufSize, 0);
}

