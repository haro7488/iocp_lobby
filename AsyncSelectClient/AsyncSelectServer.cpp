#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "CirQueue.h"

#define BUFSIZE 512
#define WM_SOCKET (WM_USER+1)

CCirQueue g_Queue;
SOCKET g_sock;


// ������ �޽��� ó�� �Լ�
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);
// ���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);
void err_display(int errcode);

enum eGameState
{
    stateNone,
    stateLogin,
    stateLoginWait,
    stateLobby,
	stateGame
};

eGameState gameState = stateNone;
ST_LOGIN login;

DWORD WINAPI Terminal(LPVOID arg)
{
	while (1)
	{
		switch (gameState)
		{
		case stateLogin:
		{
			gameState = stateLoginWait;
			printf(" - �α��� - \n");
			printf("ID : ");
			scanf("%s", login.name);
			printf("PW : ");
			scanf("%s", login.pwd);

			login.PktID = PKT_LOGIN;
			login.PktSize = sizeof(login);
			int ret = send(g_sock, (char*)&login, login.PktSize, 0);
			if (ret == SOCKET_ERROR)
			{

			}
		}
			break;
		case stateGame:
		{
			printf("���� �÷��� ���Դϴ�.\n");
			Sleep(1000);
		}
			break;
		default:
			break;
		}
		//getch();

        if (_kbhit())
        {
            printf("----------------------------\n");
            printf("1. Create room\n");
            printf("2. Exit room\n");
            printf("3. Ready\n");
            printf("4. Play\n");
            printf("----------------------------\n");
            printf("command : ");

            int command;
            scanf("%d", &command);

            switch (command)
            {
            case 1:
                {
                      printf("room name : ");

                      ST_ROOM_CREATE room;
                      room.PktID = PKT_CREATEROOM;
                      room.PktSize = sizeof(ST_ROOM_CREATE);
                      scanf("%s", room.title);

                      send(g_sock, (char *)&room, room.PktSize, 0);
                }
                break;
            }
        }
	}
}

int main(int argc, char* argv[])
{
	DWORD ThreadID;
	HANDLE hThread = CreateThread(NULL, 0, Terminal, 0, 0, &ThreadID);
	CloseHandle(hThread);

	int retval;

	// ������ Ŭ���� ���
	WNDCLASS wndclass;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hInstance = NULL;
	wndclass.lpfnWndProc = (WNDPROC)WndProc;
	wndclass.lpszClassName = "MyWindowClass";
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	if(!RegisterClass(&wndclass)) return -1;

	// ������ ����
	HWND hWnd = CreateWindow("MyWindowClass", "TCP ����",
		WS_OVERLAPPEDWINDOW, 0, 0, 600, 300,
		NULL, (HMENU)NULL, NULL, NULL);
	if(hWnd == NULL) return -1;
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return -1;

	// socket()
	g_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(g_sock == INVALID_SOCKET) err_quit("socket()");



	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(30000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	retval = connect(g_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// WSAAsyncSelect()
	retval = WSAAsyncSelect(g_sock, hWnd,
		WM_SOCKET, FD_READ | FD_CLOSE);
	if (retval == SOCKET_ERROR) err_quit("WSAAsyncSelect()");

	gameState = stateLogin;

	// �޽��� ����
	MSG msg;
	while(GetMessage(&msg, 0, 0, 0) > 0){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// ���� ����
	WSACleanup();
	return msg.wParam;
}

void SetPixel(HWND hWnd, int x, int y, COLORREF color)
{
	HDC hDC = GetDC(hWnd);
	SetPixel(hDC, x, y, color);
	ReleaseDC(hWnd, hDC);
}

// ������ �޽��� ó��
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, 
	WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		{
			MOUSEPOSITION pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			pt.PktID = PKT_MOUSPOSITION;
			pt.PktSize = sizeof(MOUSEPOSITION);

			SetPixel(hWnd, pt.x, pt.y, RGB(255, 0, 0));

			int ret = send(g_sock, (char *)&pt, pt.PktSize, 0);
			if (ret == SOCKET_ERROR)
			{
			}
		}
		break;
	case WM_SOCKET: // ���� ���� ������ �޽���
		ProcessSocketMessage(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



void PacketProcess(PACKETHEADER *pHeader, HWND hWnd)
{
	switch (pHeader->PktID)
	{
		case PKT_MOUSPOSITION:
		{
			MOUSEPOSITION *pt = (MOUSEPOSITION *)pHeader;
			SetPixel(hWnd, pt->x, pt->y, RGB(255, 0, 0));
		}
		break;
		case PKT_LOGINRESULT:
		{
			ST_LOGIN_RESULT *pt = (ST_LOGIN_RESULT*)pHeader;
			switch (pt->result)
			{
            case loginSuccess:
            {
                printf("�α��� ����\n");
                gameState = stateLoginWait;

                ST_LOBBYINFOREQ  lobbyReq;
                lobbyReq.PktID = PKT_LOBBYINFOREQ;
                lobbyReq.PktSize = sizeof(ST_LOBBYINFOREQ);
                send(g_sock, (char *)&lobbyReq, lobbyReq.PktSize, 0);
            }
				break;
			case loginFailWithNoUser:
				printf("�α��� ���� - ������ ��ϵ��� �ʾҽ��ϴ�.\n");
				gameState = stateLogin;
				break;
			case loginFailWithWorngPassward:
				printf("�α��� ���� - �н����尡 ���� �ʽ��ϴ�.\n");
				gameState = stateLogin;
				break;
            case loginFailWithExistUser:
                printf("�α��� ���� - �̹� ������ �����Դϴ�.\n");
                gameState = stateLogin;
                break;
			default:
				printf("������ �־�\n");
				break;
			}
		}
		break;
        case PKT_LOBBYINFORES:
        {
           gameState = stateLobby;

           ST_LOBBYINFORES *pName = (ST_LOBBYINFORES *)pHeader;
           printf("USER : %s\n", pName->name);
        }
         break;
        case PKT_ROOMINFO:
        {
           ST_ROOM_INFO *pRoomInfo = (ST_ROOM_INFO *)pHeader;
           printf("room : %d, %d/%d, title : %s\n", 
               pRoomInfo->index, pRoomInfo->cur, pRoomInfo->max, pRoomInfo->title);
        }
        break;
	}
}

// ���� ���� ������ �޽��� ó��
void ProcessSocketMessage(HWND hWnd, UINT uMsg, 
	WPARAM wParam, LPARAM lParam)
{
	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	int retval;
	char buf[BUFSIZE];

	// ���� �߻� ���� Ȯ��
	if(WSAGETSELECTERROR(lParam)){
		err_display(WSAGETSELECTERROR(lParam));		
		return;
	}

	// �޽��� ó��
	switch(WSAGETSELECTEVENT(lParam))
	{
	case FD_READ:
		{
			// ������ �ޱ�
			retval = recv(wParam, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK){
					err_display("recv()");
				}
				return;
			}

			int ret = g_Queue.OnPutData(buf, retval);
			if (ret > 0)
			{
				PACKETHEADER *pHeader = g_Queue.GetPacket();
				while (pHeader != NULL)
				{
					PacketProcess(pHeader, hWnd);
					
					g_Queue.OnPopData(pHeader->PktSize);
					pHeader = g_Queue.GetPacket();
				}
			}		
		}
		break;
	case FD_WRITE:
		break;
	case FD_CLOSE:
		closesocket(wParam);
		break;
	}
}


// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

// ���� �Լ� ���� ���
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[����] %s", (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}