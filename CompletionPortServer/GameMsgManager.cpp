//============================================================================//
// GameMsgManager.cpp

#include "GameMsgManager.h"
#include "GameProc.h"
#include <time.h>


CGameMsgManager	*CGameMsgManager::m_pGameManager = NULL;
//---------------------------------------------------------------------------
//
CGameMsgManager::CGameMsgManager()
{
	m_pGameManager = this;

	m_queGameMsgContainer = new CArrayListContainer<GAME_MSG>(MAX_GAME_MSG);
	m_queGameMsg = new CArrayList<GAME_MSG>(MAX_GAME_MSG);

    DWORD ThreadId;
    HANDLE hThread = CreateThread(NULL, 0, GameMsgLoop, 0, 0, &ThreadId);
    CloseHandle(hThread);
}

//---------------------------------------------------------------------------
//
CGameMsgManager::~CGameMsgManager()
{
	delete m_queGameMsg; m_queGameMsg = NULL;
	delete m_queGameMsgContainer; m_queGameMsgContainer = NULL;
}

//---------------------------------------------------------------------------
//
void CGameMsgManager::SendGameMessage(DWORD msg, DWORD wParam, DWORD lParam, char *pPacket)
{
	GAME_MSG *pMsg = m_queGameMsgContainer->pop();

	pMsg->msg = msg;
	pMsg->wParam = wParam;
	pMsg->lParam = lParam;

	if(pPacket != NULL)
	   memcpy(pMsg->packet, pPacket, wParam);

	m_queGameMsg->push(pMsg);
}

//---------------------------------------------------------------------------
// message loop
DWORD WINAPI CGameMsgManager::GameMsgLoop(LPVOID pArg)
{
	GAME_MSG *pMsg;

	srand( (unsigned)time( NULL ) ); //rand() 함수 초기화.

	while(1)
	{
		if( (pMsg = g_pGameMgr->GetGameMessage()) != NULL)
		{			
			if(pMsg->msg == GM_QUIT)
			{	
				g_pGameMgr->DefaultGameMsgProc(pMsg);
				break;
			}

			OnReceivePacket(pMsg->msg, pMsg->wParam, pMsg->lParam, pMsg->packet);

			g_pGameMgr->DefaultGameMsgProc(pMsg);
		}
		
	}

	ExitThread(0);

	return 0;
}