//============================================================================//
// GameMsgManager.h

#ifndef _GAMEMSGMANAGER_H_
#define _GAMEMSGMANAGER_H_


#include "array_list.h"
#include "array_list_container.h"
#include "global.h"

#define MAX_GAME_MSG_SIZE   512//1008
#define MAX_GAME_MSG        4000//150//30000//10240 //유저수에 따라 크기를 늘려줘야 함

#define GM_PKTRECEIVE   1
#define GM_QUIT                2
#define GM_GAMETIMER    3
#define GM_DISCONNECTUSER   4

typedef struct _tgGAME_MSG
{
	DWORD msg;
	DWORD wParam;
	DWORD lParam;
	char  packet[MAX_GAME_MSG_SIZE];
}GAME_MSG;

class CGameMsgManager
{
public:
	static CGameMsgManager		 *m_pGameManager;

protected:
	CArrayList<GAME_MSG>          *m_queGameMsg;
	CArrayListContainer<GAME_MSG> *m_queGameMsgContainer; 

public:
	CGameMsgManager();
	~CGameMsgManager();

	GAME_MSG *GetGameMessage() { return m_queGameMsg->pop(); }
	void DefaultGameMsgProc(GAME_MSG *pMsg) { m_queGameMsgContainer->push(pMsg); }

	void SendGameMessage(DWORD msg, DWORD wParam, DWORD lParam, char *pPacket);

	static DWORD WINAPI GameMsgLoop(LPVOID pArg);
};



#endif