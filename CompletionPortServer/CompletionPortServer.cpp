#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#include "UserManager.h"
#include "GameMsgManager.h"
#include "RoomManager.h"
#define BUFSIZE 512

// 소켓 입출력 함수
DWORD WINAPI WorkerThread(LPVOID arg);
// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);

BOOL RegisterCompletionPort(SOCKET hSocket, DWORD key);
BOOL NotifyCompletionStatus(DWORD key, DWORD dwBytesTransferred, WSAOVERLAPPED* pOverlapped, int* pErrCode);

HANDLE g_hCP;
CUserManager *g_pUserMgr;
CGameMsgManager *g_pGameMgr;
CRoomManager *g_pRoomMgr;

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return -1;

	// 입출력 완료 포트 생성
	g_hCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(g_hCP == NULL) return -1;

	// CPU 개수 확인
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// (CPU 개수 * 2)개의 작업자 스레드 생성
	HANDLE hThread;
	DWORD ThreadId;
	for(int i=0; i<(int)si.dwNumberOfProcessors*2; i++)
	{
		hThread = CreateThread(NULL, 0, WorkerThread, g_hCP, 0, &ThreadId);
		if(hThread == NULL) return -1;
		CloseHandle(hThread);
	}

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(30000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");
	
	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	g_pUserMgr = new CUserManager();
	g_pGameMgr = new CGameMsgManager();
	g_pRoomMgr = new CRoomManager();

	DWORD iUserID = 1;
	while(1)
	{
		// accept()
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		SOCKET client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			continue;
		}
		printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",  inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		CUser *pUser = g_pUserMgr->GetListReadyUser();
		if (pUser)
		{
			pUser->InitUser(iUserID, client_sock, clientaddr);
            g_pUserMgr->OnAddUser(iUserID, pUser);
		}

		iUserID++;

		// 소켓과 입출력 완료 포트 연결
		//HANDLE hResult = CreateIoCompletionPort((HANDLE)client_sock, g_hCP, (DWORD)client_sock, 0);
		//if(hResult == NULL) return -1;		
	}

	delete g_pUserMgr;
	delete g_pGameMgr;
	delete g_pRoomMgr;
	// 윈속 종료
	WSACleanup();
	return 0;
}

BOOL RegisterCompletionPort(SOCKET hSocket, DWORD key)
{
	HANDLE hResult = CreateIoCompletionPort((HANDLE)hSocket, g_hCP, key, 0);
	if (hResult == NULL) return FALSE;

	return TRUE;
}

BOOL NotifyCompletionStatus(DWORD key, DWORD dwBytesTransferred, WSAOVERLAPPED* pOverlapped, int* pErrCode)
{
	BOOL ret = PostQueuedCompletionStatus(g_hCP, dwBytesTransferred, key, pOverlapped);
	
	if (!ret && pErrCode != NULL)
	{
		*pErrCode = GetLastError();
	}

	return ret;
}


DWORD WINAPI WorkerThread(LPVOID arg)
{
	HANDLE hcp = (HANDLE)arg;
	int retval;
	
	while(1)
	{
		// 비동기 입출력 완료 기다리기
		DWORD cbTransferred;
		SOCKET client_sock;
		PER_IO_OVERLAPPED *ptr;
		CUser *pUser;
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, (LPDWORD)&pUser, (LPOVERLAPPED *)&ptr, INFINITE);

		// 클라이언트 정보 얻기
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(pUser->GetSocket(), (SOCKADDR *)&clientaddr, &addrlen);
		
		// 비동기 입출력 결과 확인
		if(retval == 0 || cbTransferred == 0)
		{
			if(retval == 0)
			{
				err_display("WSAGetOverlappedResult()");
			}

			g_pUserMgr->OnDeleteUser(pUser->GetUserID(), pUser);
			pUser->CloseUserSocket();
			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", 
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

			continue;
		}
		
		if (ptr->io_type == IO_RECV) //recv
		{
			if (pUser->OnSendGamePacket(cbTransferred) == FALSE)
			{
				g_pUserMgr->OnDeleteUser(pUser->GetUserID(), pUser);
				pUser->CloseUserSocket();
			}
		}
		else if (ptr->io_type == IO_SEND) //send
		{
		}
		else
		{
			g_pUserMgr->OnDeleteUser(pUser->GetUserID(), pUser);
			pUser->CloseUserSocket();
		}
	}

	return 0;
}

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}