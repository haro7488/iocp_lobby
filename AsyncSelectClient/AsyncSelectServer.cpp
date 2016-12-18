#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "CirQueue.h"
#include <string>

#define BUFSIZE 512
#define WM_SOCKET (WM_USER+1)

CCirQueue g_Queue;
SOCKET g_sock;


// 윈도우 메시지 처리 함수
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);

void EnterIsContinue();
// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);
void err_display(int errcode);

enum eGameState
{
	stateNone,
	stateLogin,
	stateLoginWait,
	stateLobby,
	stateWaitCreateRoom,
	stateWaitEnterRoom,
	stateWaitExitRoom,
	stateRoom,
	stateEnterGame,
	stateGame
};

eGameState gameState = stateNone;
ST_LOGIN login;

using std::string;

class CUser
{
public:
	string name;
};

class CRoom
{
public:

};

CUser g_User;

DWORD WINAPI Terminal(LPVOID arg)
{
	while (1)
	{
		switch (gameState)
		{
		case stateLogin:
		{
			system("cls");
			gameState = stateLoginWait;
			printf(" - 로그인 - \n");
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
		case stateLobby:
		{
			printf(" - 로비 - \n");
			printf("ME : %s\n\n", g_User.name.c_str());
			printf("방목록 = \n");
			ST_LOBBYINFOREQ  lobbyReq;
			lobbyReq.PktID = PKT_LOBBYINFOREQ;
			lobbyReq.PktSize = sizeof(ST_LOBBYINFOREQ);
			send(g_sock, (char *)&lobbyReq, lobbyReq.PktSize, 0);
			Sleep(500);
		}
		break;
		case stateWaitCreateRoom:
		{
			continue;
		}
		break;
		case stateWaitEnterRoom:
		{
			continue;
		}
		break;
		case stateRoom:
		{
			printf(" - 룸 - \n");
			printf("ME : %s\n\n", g_User.name.c_str());
			printf("대기자 = \n");
			ST_ROOMINFOREQ roomReq;
			roomReq.PktID = PKT_ROOMINFOREQ;
			roomReq.PktSize = sizeof(ST_ROOMINFOREQ);
			send(g_sock, (char*)&roomReq, roomReq.PktSize, 0);

			Sleep(500);
		}
		break;
		case stateGame:
		{
			printf("게임 플레이 중입니다.\n");
			Sleep(1000);
		}
		break;
		default:
			break;
		}
		//getch();

		if (gameState == stateLogin || gameState == stateLoginWait || gameState == stateWaitEnterRoom)
			continue;

        if (_kbhit())
        {
            printf("----------------------------\n");
            printf("1. Create Room\n");
			printf("2. Enter Room\n");
            printf("3. Exit Room\n");
            printf("4. Ready\n");
            printf("5. Play\n");
            printf("----------------------------\n");
            printf("command : ");

            int command;
            scanf("%d", &command);

            switch (command)
            {
            case 1: // Create Room
			{
				if (gameState != stateLobby)
					break;
		
                printf("만들 방 이름 : ");

                ST_ROOM_CREATE_REQ room;
                room.PktID = PKT_CREATEROOMREQ;
                room.PktSize = sizeof(ST_ROOM_CREATE_REQ);
                scanf("%s", room.title);

                send(g_sock, (char *)&room, room.PktSize, 0);
				gameState = stateWaitCreateRoom;
            }
            break;
			case 2: // Enter Room
			{
				if (gameState != stateLobby)
					break;
				
				ST_ENTER_ROOM_REQ room;
				room.PktID = PKT_ENTERROOMREQ;
				room.PktSize = sizeof(ST_ENTER_ROOM_REQ);
				printf("들어갈 방 번호 : ");
				scanf("%d", room.roomNum);

				send(g_sock, (char *)&room, room.PktSize, 0);

				gameState = stateWaitEnterRoom;
			}

			break;
			case 3: // Exit Room
			{
				if (gameState != stateRoom)
					break;

				gameState = stateWaitExitRoom;
			}
			break;
			case 4: // Ready
			{
				if (gameState != stateRoom)
					break;

			}
			break;
			case 5: // Play
			{
				if (gameState != stateRoom)
					break;

				gameState = stateEnterGame;
			}
			break;
            }
        }

		system("cls");
	}
}

int main(int argc, char* argv[])
{
	DWORD ThreadID;
	HANDLE hThread = CreateThread(NULL, 0, Terminal, 0, 0, &ThreadID);
	CloseHandle(hThread);

	int retval;

	// 윈도우 클래스 등록
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

	// 윈도우 생성
	HWND hWnd = CreateWindow("MyWindowClass", "TCP 서버",
		WS_OVERLAPPEDWINDOW, 0, 0, 600, 300,
		NULL, (HMENU)NULL, NULL, NULL);
	if(hWnd == NULL) return -1;
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	// 윈속 초기화
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

	// 메시지 루프
	MSG msg;
	while(GetMessage(&msg, 0, 0, 0) > 0){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 윈속 종료
	WSACleanup();
	return msg.wParam;
}

void SetPixel(HWND hWnd, int x, int y, COLORREF color)
{
	HDC hDC = GetDC(hWnd);
	SetPixel(hDC, x, y, color);
	ReleaseDC(hWnd, hDC);
}

// 윈도우 메시지 처리
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
	case WM_SOCKET: // 소켓 관련 윈도우 메시지
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
				gameState = stateLoginWait;
                printf("로그인 성공\n");
				g_User.name = login.name;

                ST_LOBBYINFOREQ  lobbyReq;
                lobbyReq.PktID = PKT_LOBBYINFOREQ;
                lobbyReq.PktSize = sizeof(ST_LOBBYINFOREQ);
                send(g_sock, (char *)&lobbyReq, lobbyReq.PktSize, 0);

				EnterIsContinue();
				gameState = stateLobby;
            }
				break;
			case loginFailWithNoUser:
				gameState = stateLoginWait;
				printf("로그인 실패 - 유저가 등록되지 않았습니다.\n");
				EnterIsContinue();
				gameState = stateLogin;
				break;
			case loginFailWithWorngPassward:
				gameState = stateLoginWait;
				printf("로그인 실패 - 패스워드가 맞지 않습니다.\n");
				EnterIsContinue();
				gameState = stateLogin;
				break;
            case loginFailWithExistUser:
				gameState = stateLoginWait;
				printf("로그인 실패 - 이미 접속한 유저입니다.\n");
				EnterIsContinue();
				gameState = stateLogin;
                break;
			default:
				gameState = stateLoginWait;
				printf("문제가 있어\n");
				EnterIsContinue();
				gameState = stateLogin;
				break;
			}
		}
		break;
        case PKT_LOBBYINFORES:
        {
			if (gameState == stateWaitExitRoom)
				gameState = stateLobby;

			ST_LOBBYINFORES *pName = (ST_LOBBYINFORES *)pHeader;
			printf("USER : %s\n", pName->name);
		}
		break;
		case PKT_LOBBYENDOFROOMINFO:
		{
			printf("\n대기자 = \n");
		}
		break;
		case PKT_CREATEROOMRES:
		{
			if (gameState == stateWaitCreateRoom)
			{
				ST_ROOM_CREATE_RES* pRoomCreateRes = (ST_ROOM_CREATE_RES*)pHeader;

				ST_ENTER_ROOM_REQ enterRoomReq;
				enterRoomReq.PktID = PKT_ENTERROOMREQ;
				enterRoomReq.PktSize = sizeof(ST_ENTER_ROOM_REQ);
				enterRoomReq.roomNum = pRoomCreateRes->roomNum;
				send(g_sock, (char *)&enterRoomReq, enterRoomReq.PktSize, 0);
				gameState = stateWaitEnterRoom;
			}
		}
		break;
		case PKT_ENTERROOMRES:
		{
			if (gameState == stateWaitEnterRoom)
			{
				ST_ENTER_ROOM_RES *pRoomResult = (ST_ENTER_ROOM_RES*)pHeader;
				switch (pRoomResult->result)
				{
				case EnterRoomSuccess:
				{
					printf("방 참가 성공\n");
					EnterIsContinue();
					gameState = stateRoom;
				}
				break;
				case EnterRoomFailWithFull:
				{
					printf("방 참가 실패 - 방이 꽉 차서 들어갈 수 없습니다.\n");
					EnterIsContinue();
					gameState = stateLobby;
				}
				break;
				default:
					break;
				}
			}
		}
		break;
        case PKT_ROOMINFORES:
        {
			if (gameState == stateWaitEnterRoom)
				gameState = stateRoom;
			ST_ROOM_INFORES *pRoomInfo = (ST_ROOM_INFORES *)pHeader;
			printf("room : %d / [%d/%d] / title : %s / master : %s \n", 
			pRoomInfo->index, pRoomInfo->cur, pRoomInfo->max, pRoomInfo->title, pRoomInfo->masterName);

        }
        break;
	}
}

// 소켓 관련 윈도우 메시지 처리
void ProcessSocketMessage(HWND hWnd, UINT uMsg, 
	WPARAM wParam, LPARAM lParam)
{
	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	int retval;
	char buf[BUFSIZE];

	// 오류 발생 여부 확인
	if(WSAGETSELECTERROR(lParam)){
		err_display(WSAGETSELECTERROR(lParam));		
		return;
	}

	// 메시지 처리
	switch(WSAGETSELECTEVENT(lParam))
	{
	case FD_READ:
		{
			// 데이터 받기
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

void EnterIsContinue()
{
	printf("계속 하려면 Enter\n");
	fflush(stdin);
	getch();
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

// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[오류] %s", (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}