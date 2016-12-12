#pragma once

#include <winsock2.h>
#include <windows.h>

#define SAFE_DELETE(p) { if(p) { delete (p); (p) = NULL;  }  }

#define IO_NONE    0
#define IO_RECV    1
#define IO_SEND    2



typedef struct _PER_IO_OVERLAPPED : public WSAOVERLAPPED
{
	DWORD  io_type;
}PER_IO_OVERLAPPED, *PPER_IO_OVERLAPPED;


class CUserManager;
extern CUserManager *g_pUserMgr;

class CGameMsgManager;
extern CGameMsgManager *g_pGameMgr;

class CRoomManager;
extern CRoomManager *g_pRoomMgr;

extern BOOL RegisterCompletionPort(SOCKET hSocket, DWORD key);
extern BOOL NotifyCompletionStatus(DWORD key, DWORD dwBytesTransferred, WSAOVERLAPPED* pOverlapped, int* pErrCode);
