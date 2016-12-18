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

#include "myHeader.h"

using namespace Gdiplus;

//#define BUFSIZE 512
//#define WM_SOCKET (WM_USER+1)

Graphics *g_BackBuffer;
Graphics *g_MainBuffer;
Bitmap *g_pBackBufferBitmap;

CPlayer *g_pPlayer1;
CPlayer *g_pPlayer2;

//CCirQueue g_Queue;
//SOCKET g_sock;

Image* imgNum[10];
Image* imgLeftWin;
Image* imgRightWin;
Image* imgRestart;

bool g_isLeftPlayer = false;
bool g_isGameOver = false;
bool g_isLeftWin = false;
bool g_isPlayingGame = false;

extern bool g_isGameOver;
extern bool g_isLeftWin;

// 윈도우 메시지 처리 함수
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);

void EnterIsContinue();
// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);
void err_display(int errcode);


///게임플레이
#define PLAYER_SIZE	50
void OnUpdate(HWND hWnd, DWORD tick);

void StartGamePlay();
void EndGamePlay();
void Init();
void DrawGameOver();
///

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
	stateWaitEnterGame,
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
			fflush(stdin);
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
		case stateWaitEnterGame:
		{

		}
		break;
		case stateGame:
		{
			printf(" - 게임 플레이 중입니다. - \n");
			printf("ME : %s\n\n", g_User.name.c_str());
			printf("대기자 = \n");
			ST_ROOMINFOREQ roomReq;
			roomReq.PktID = PKT_ROOMINFOREQ;
			roomReq.PktSize = sizeof(ST_ROOMINFOREQ);
			send(g_sock, (char*)&roomReq, roomReq.PktSize, 0);
			Sleep(1000);
		}
		break;
		default:
			break;
		}
		//getch();

		//if (gameState == stateLogin || gameState == stateLoginWait || gameState == stateWaitEnterRoom)
		//	continue;

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
				scanf("%d", &room.roomNum);

				send(g_sock, (char *)&room, room.PktSize, 0);

				gameState = stateWaitEnterRoom;
			}

			break;
			case 3: // Exit Room
			{
				if (gameState != stateRoom)
					break;

				ST_EXIT_ROOM_REQ room;
				room.PktID = PKT_EXITROOMREQ;
				room.PktSize = sizeof(ST_EXIT_ROOM_REQ);
				send(g_sock, (char *)&room, room.PktSize, 0);

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

				ST_START_GAME_REQ startGameReq;
				startGameReq.PktID = PKT_STARTGAMEREQ;
				startGameReq.PktSize = sizeof(ST_START_GAME_REQ);
				send(g_sock, (char*)&startGameReq, startGameReq.PktSize, 0);
				
				gameState = stateWaitEnterGame;
			}
			break;
			}
		}
		if(!(gameState == stateLoginWait ||
			gameState == stateWaitCreateRoom ||
			gameState == stateWaitEnterRoom ||
			gameState == stateWaitExitRoom ||
			gameState == stateWaitEnterGame))
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
	if (!RegisterClass(&wndclass)) return -1;

	// 윈도우 생성
	HWND hWnd = CreateWindow("MyWindowClass", "TCP 서버",
		WS_OVERLAPPEDWINDOW, 0, 0, 1000, 600,
		NULL, (HMENU)NULL, NULL, NULL);
	if (hWnd == NULL) return -1;
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	////GDIPlus 초기화
	GdiplusStartupInput			gdiplusStartupInput;
	ULONG_PTR					gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	RECT rc;
	GetClientRect(hWnd, &rc);

	g_pBackBufferBitmap = new Bitmap(rc.right - rc.left, rc.bottom - rc.top);
	g_BackBuffer = new Graphics(g_pBackBufferBitmap);
	g_BackBuffer->SetPageUnit(Gdiplus::Unit::UnitPixel);

	HDC hDC = GetDC(hWnd);
	g_MainBuffer = new Graphics(hDC);
	g_MainBuffer->SetPageUnit(Gdiplus::Unit::UnitPixel);
	////
	
	////이미지 초기화
	imgNum[0] = new Image(L"Images/n_00.png");
	imgNum[1] = new Image(L"Images/n_01.png");
	imgNum[2] = new Image(L"Images/n_02.png");
	imgNum[3] = new Image(L"Images/n_03.png");
	imgNum[4] = new Image(L"Images/n_04.png");
	imgNum[5] = new Image(L"Images/n_05.png");
	imgNum[6] = new Image(L"Images/n_06.png");
	imgNum[7] = new Image(L"Images/n_07.png");
	imgNum[8] = new Image(L"Images/n_08.png");
	imgNum[9] = new Image(L"Images/n_09.png");

	imgLeftWin = new Image(L"Images/LeftWin.png");
	imgRightWin = new Image(L"Images/RightWin.png");
	imgRestart = new Image(L"Images/Restart.png");
	////



	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	g_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_sock == INVALID_SOCKET) err_quit("socket()");



	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(30000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	retval = connect(g_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// WSAAsyncSelect()
	retval = WSAAsyncSelect(g_sock, hWnd,
		WM_SOCKET, FD_READ | FD_CLOSE);
	if (retval == SOCKET_ERROR) err_quit("WSAAsyncSelect()");

	gameState = stateLogin;

	////
	DWORD tick = GetTickCount();
	// 메시지 루프
	MSG msg;
	//while (GetMessage(&msg, 0, 0, 0) > 0) {
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	//}

	////메세지 루프
	while (1)
	{
		//윈도우 메세지가 있을경우 메세지를 처리한다.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else //메세지가 없을 경우 게임 루프를 실행한다.
		{
			DWORD curTick = GetTickCount();
			OnUpdate(hWnd, curTick - tick);
			tick = curTick;

			g_MainBuffer->DrawImage(g_pBackBufferBitmap, 0, 0);
		}
	}
	////

	////메모리 해제
	delete g_pPlayer1;
	delete g_pPlayer2;

	delete g_BackBuffer;
	delete g_MainBuffer;
	delete g_pBackBufferBitmap;

	for (int i = 0; i < 10; i++)
	{
		delete imgNum[i];
	}
	delete imgLeftWin;
	delete imgRightWin;
	////

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
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	{
		/*
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
		*/
	}
	break;
	case WM_KEYDOWN:
	{
		if (!g_isPlayingGame)
			break;

		CPlayer* player = g_isLeftPlayer ? g_pPlayer1 : g_pPlayer2;

		switch (wParam)
		{
		case 'W':
		{
			if (player)
			{
				player->Move(0, -5);
			}
		}
		break;
		case 'S':
		{
			if (player)
			{
				player->Move(0, 5);
			}
		}
		break;
		case VK_SHIFT:
		{
			if (player)
			{
				player->Shoot();
			}
		}
		break;
		case VK_RETURN:
		{
			if (player)
			{
				player->Boom();
			}

			if (g_isGameOver)
			{
				Init();
				PACKETHEADER restart;
				restart.PktID = PKT_RESTART;
				restart.PktSize = sizeof(restart);
				int ret = send(g_sock, (char*)&restart, restart.PktSize, 0);
				if (ret == SOCKET_ERROR)
				{

				}
			}
		}
		break;
		case VK_SPACE:
		{
			if (player)
			{
				player->BeginSuperGuard();
				player->SendSuperGuard();
			}
		}
		break;
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
	CPlayer* player = g_isLeftPlayer ? g_pPlayer2 : g_pPlayer1;
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
			g_isLeftPlayer = true;
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
			case EnterRoomFailByNoRoom:
			{
				printf("방 참가 실패 - 방이 존재하지 않습니다.\n");
				EnterIsContinue();
				gameState = stateLobby;
			}
			break;
			case EnterRoomFailByFull:
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
		printf("방번호 : %d / [%d/%d] / 방이름 : %s / 방장 : %s \n",
			pRoomInfo->index, pRoomInfo->cur, pRoomInfo->max, pRoomInfo->title, pRoomInfo->masterName);
	}
	break;
	case PKT_EXITROOMRES:
	{
		if (!(gameState == stateWaitExitRoom || gameState == stateRoom || gameState == stateGame))
			break;
		g_isLeftPlayer = false;
		EndGamePlay();
		printf("방에서 나갑니다.\n");
		EnterIsContinue();
		gameState = stateLobby;
	}
	break;
	case PKT_STARTGAMERES:
	{
		if (!(gameState == stateWaitEnterGame || gameState == stateRoom))
			break;
		StartGamePlay();
		gameState = stateGame;
	}
	break;
	case PKT_ENDGAMERES:
	{
		EndGamePlay();
		gameState = stateRoom;
	}
	break;

	////GamePlay Process
	case PKT_MOVE:
	{
		POSITIONINFO* posInfo = (POSITIONINFO*)pHeader;
		player->SetPosition(posInfo->x, posInfo->y);
	}
	break;
	case PKT_SHOOT:
	{
		POSITIONINFO* posInfo = (POSITIONINFO*)pHeader;
		player->AddBullet(posInfo->x, posInfo->y);
	}
	break;
	case PKT_BOOM:
	{
		POSITIONINFO* posInfo = (POSITIONINFO*)pHeader;
		player->AddBoom(posInfo->x, posInfo->y);
	}
	break;
	case PKT_ENEMYSUPERGUARD:
	{
		player->BeginSuperGuard();
	}
	break;
	case PKT_RESTART:
	{
		Init();
	}
	break;
	////
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
	if (WSAGETSELECTERROR(lParam)) {
		err_display(WSAGETSELECTERROR(lParam));
		return;
	}

	// 메시지 처리
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_READ:
	{
		// 데이터 받기
		retval = recv(wParam, buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
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
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
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
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
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
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[오류] %s", (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void OnUpdate(HWND hWnd, DWORD tick)
{
	if (tick < 10)
		return;

	if (g_isGameOver)
	{
		DrawGameOver();
		return;
	}

	Color color(255, 255, 255);
	g_BackBuffer->Clear(color);

	if (g_pPlayer1)
	{
		g_pPlayer1->DrawUIHP(g_BackBuffer, imgNum);
		g_pPlayer1->Draw(g_BackBuffer, g_pPlayer2, tick);
	}

	if (g_pPlayer2)
	{
		g_pPlayer2->DrawUIHP(g_BackBuffer, imgNum);
		g_pPlayer2->Draw(g_BackBuffer, g_pPlayer1, tick);
	}


	//g_BackBuffer->FillRectangle(g_brush, rect);
}

void StartGamePlay()
{
	Init();
	g_isPlayingGame = true;
}

void EndGamePlay()
{
	g_isPlayingGame = false;
	if (g_pPlayer1 != nullptr)
		delete g_pPlayer1;
	if (g_pPlayer2 != nullptr)
		delete g_pPlayer2;

}

void Init()
{
	if (g_pPlayer1 != nullptr)
		delete g_pPlayer1;
	if (g_pPlayer2 != nullptr)
		delete g_pPlayer2;

	g_pPlayer1 = new CPlayer(0, 300, PLAYER_SIZE, PLAYER_SIZE, Color::Blue);
	g_pPlayer2 = new CPlayer(950, 300, PLAYER_SIZE, PLAYER_SIZE, Color::Red);

	g_pPlayer1->m_Direction = DIRECTION_LEFT;
	g_pPlayer1->CreateSubPlayer();
	g_pPlayer2->m_Direction = DIRECTION_RIGHT;
	g_pPlayer2->CreateSubPlayer();
	g_isGameOver = false;
}

void DrawGameOver()
{
	if (g_isLeftWin)
	{
		g_BackBuffer->DrawImage(imgLeftWin, 500 - 250, 300 - 50, 500, 100);
	}
	else
	{
		g_BackBuffer->DrawImage(imgRightWin, 500 - 250, 300 - 50, 500, 100);
	}
	g_BackBuffer->DrawImage(imgRestart, 500 - 250, 380 - 50, 500, 100);
}
