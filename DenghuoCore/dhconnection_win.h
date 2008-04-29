// Denghuo
// Aurthor: Liang Tang
// Update: 08-04-14
#ifndef _DENGHUO_CORE_CONNECTION_H_
#define _DENGHUO_CORE_CONNECTION_H_

#include <winsock2.h>
#include <ws2tcpip.h>


// Definitions of states of the socket
#define DHCONN_STATE_NOTCONNECTED 0
#define DHCONN_STATE_WAIT         1
#define DHCONN_STATE_READY_READ   2
#define DHCONN_STATE_READY_WRITE  4

typedef struct _dhConnectionSet
{
	int          nMaxConn;
	int          nUsedConnCount;
	SOCKET*      pSockets;
	SOCKADDR_IN* pServerAddrs;
	int*         pStates;
}dhConnectionSet;

int dhConn_startup(void);
int dhConn_cleanup(void);
int dhConn_create(dhConnectionSet** ppConnSet, int nMaxConn);
int dhConn_release(dhConnectionSet* pConnSet);
int dhConn_connect(dhConnectionSet* pConnSet, const char* pszConn);
int dhConn_disconnect(dhConnectionSet* pConnSet, int nIndex);
int dhConn_disconnectAll(dhConnectionSet* pConnSet);
int dhConn_usedcount(dhConnectionSet* pConnSet);
int dhConn_wait(dhConnectionSet* pConnSet);
int dhConn_canwrite(dhConnectionSet* pConnSet, int nIndex);
int dhConn_canread(dhConnectionSet* pConnSet, int nIndex);
int dhConn_canconnect(dhConnectionSet* pConnSet, int nIndex);
int dhConn_write(dhConnectionSet* pConnSet, int nIndex, const char* pBuf, int nBufSize);
int dhConn_read(dhConnectionSet* pConnSet, int nIndex, char* pBuf, int nBufSize);

#endif

