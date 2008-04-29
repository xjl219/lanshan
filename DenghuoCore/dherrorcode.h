// Denghuo
// Aurthor: Liang Tang
// Update: 08-04-14

#ifndef _DENGHUO_CORE_ERROR_CODE_H_
#define _DENGHUO_CORE_ERROR_CODE_H_
#include <assert.h>

#define DHERR_NONE 0
#define DHERR_UNKNOWN -1

#define DHERR_INDEX_OUF_OF_BOUND -2
#define DHERR_PARAMETER_FORMAT -10


#define DHERR_FILE_CANNOT_OPEN -50

#define DHERR_SOCKET_ERROR -100
#define DHERR_SOCKET_NOT_AVAILABLE -101
#define DHERR_SOCKET_CANNOT_GET_HOSTNAME -102

#define DH_ON_FAILED_RETURN(nRet) \
	{ \
	if(nRet < 0) { \
	assert(0); \
	return nRet; }  \
	}

#define DH_IF_FAILED(nRet) nRet < 0 
#define DH_IF_SUCCESS(nRet) nRet >= 0


#endif
