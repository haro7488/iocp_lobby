#include "Room.h"
#include "UserManager.h"


CRoom::CRoom()
{
}


CRoom::~CRoom()
{
}
CUser *CRoom::FindUser(DWORD user_id)
{
	
	CUser *pUser = NULL;
	MAP_USER::iterator  iterator_user;
	iterator_user = m_mapUser.find(user_id);

	if (iterator_user != m_mapUser.end())
	{
		pUser = (CUser *)iterator_user->second;
		
		return pUser;
	}

	return NULL;
}
void CRoom::QuitAllUser()
{
	MAP_USER::iterator iterator_user = m_mapUser.begin();
	CUser* user;
	for (; iterator_user != m_mapUser.end(); )
	{

		user = (iterator_user->second);
		user->QuitRoom();
		iterator_user = m_mapUser.begin();
	}
}
eEnterRoomResult CRoom::OnAddUser(DWORD user_id, CUser *pUser)		// 방에 유저 들어옴
{
    if (m_curUser >= m_maxUser)
        return EnterRoomFailByFull;

	if (FindUser(user_id))
	{
		return EnterRoomFailByExistUser;
	}

    m_curUser++;

	m_mapUser.insert(std::make_pair(user_id, pUser));

    return EnterRoomSuccess;
}
void CRoom::OnDeleteAddUser(DWORD user_id, CUser *pUser)	// 유저 나감
{
	if (pUser == NULL) return;

	m_curUser--;

	MAP_USER::iterator iterator_user = m_mapUser.find(user_id);

	if (iterator_user != m_mapUser.end())
	{
		m_mapUser.erase(iterator_user);
	}
#ifdef _DEBUG
	else
	{
		//OutputDebugString("없는 유저를 지운다...\n");
	}
#endif	

}
//--------------------------------- 패킷 송신 관련 ---------------------------------//
BOOL CRoom::OnSendAllUserRoomInfo()
{
    ST_ROOM_INFORES *pRoomInfo = (ST_ROOM_INFORES *)m_SendBuff;
    pRoomInfo->PktID = PKT_ROOMINFORES;
    pRoomInfo->PktSize = sizeof(ST_ROOM_INFORES);
    pRoomInfo->max = m_maxUser;
    pRoomInfo->cur = m_curUser;
    pRoomInfo->index = m_iRoom;
    strcpy(pRoomInfo->title, m_strRoomName.c_str());

    m_WSABUF.len = pRoomInfo->PktSize;

    g_pUserMgr->OnSendAllUser(&m_WSABUF);

    return TRUE;
}

BOOL CRoom::OnSendRoomInfo(CUser * pUser)
{
	ST_ROOM_INFORES *pRoomInfo = (ST_ROOM_INFORES *)m_SendBuff;
	pRoomInfo->PktID = PKT_ROOMINFORES;
	pRoomInfo->PktSize = sizeof(ST_ROOM_INFORES);
	pRoomInfo->max = m_maxUser;
	pRoomInfo->cur = m_curUser;
	pRoomInfo->index = m_iRoom;
	strcpy(pRoomInfo->title, m_strRoomName.c_str());

	m_WSABUF.len = pRoomInfo->PktSize;

	pUser->OnSendPacket(&m_WSABUF);

	return TRUE;
}

BOOL CRoom::OnSendInRoomInfo(CUser * pUser)
{
	ST_ROOM_INFORES *pRoomInfo = (ST_ROOM_INFORES *)m_SendBuff;
	pRoomInfo->PktID = PKT_ROOMINFORES;
	pRoomInfo->PktSize = sizeof(ST_ROOM_INFORES);
	pRoomInfo->max = m_maxUser;
	pRoomInfo->cur = m_curUser;
	pRoomInfo->index = m_iRoom;
	strcpy(pRoomInfo->title, m_strRoomName.c_str());
	strcpy(pRoomInfo->masterName, m_strRoomMasterName.c_str());

	m_WSABUF.len = pRoomInfo->PktSize;

	pUser->OnSendPacket(&m_WSABUF);

	MAP_USER::iterator  iterator_user = m_mapUser.begin();
	CUser* user;
	for (; iterator_user != m_mapUser.end(); iterator_user++)
	{
		user = (iterator_user->second);
		pUser->OnSendUserName(user);
	}

	return TRUE;
}



BOOL CRoom::OnSendPacket()
{
	

	return TRUE;
}


//---------------------------------------------------------------------------------------------
BOOL CRoom::OnSendPacket(WSABUF *pWsaBuff)
{
	
	return TRUE;
}

BOOL CRoom::OnSendStartGame()
{
	MAP_USER::iterator  iterator_user = m_mapUser.begin();
	CUser* pUser;
	for (; iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);
		pUser->OnSendStartGame();
	}

	return TRUE;
}

BOOL CRoom::OnSendEndGame()
{
	MAP_USER::iterator  iterator_user = m_mapUser.begin();
	CUser* pUser = nullptr;
	for (; iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);
		if (pUser != nullptr)
			pUser->OnSendEndGame();
	}

	return TRUE;
}

BOOL CRoom::OnSendGamePlayMove(CUser * sender, POSITIONINFO* posInfo)
{
	MAP_USER::iterator  iterator_user = m_mapUser.begin();
	CUser* pUser = nullptr;
	CUser* pEnemy = nullptr;
	for (; iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);
		if (sender->GetUserID() != pUser->GetUserID())
			pEnemy = pUser;
	}
	pEnemy->OnSendGamePlayMove(posInfo);

	return TRUE;
}

BOOL CRoom::OnSendGamePlayShoot(CUser * sender, POSITIONINFO* posInfo)
{
	MAP_USER::iterator  iterator_user = m_mapUser.begin();
	CUser* pUser = nullptr;
	CUser* pEnemy = nullptr;
	for (; iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);
		if (sender->GetUserID() != pUser->GetUserID())
			pEnemy = pUser;
	}
	pEnemy->OnSendGamePlayShoot(posInfo);

	return TRUE;
}

BOOL CRoom::OnSendGamePlayBoom(CUser * sender, POSITIONINFO* posInfo)
{
	MAP_USER::iterator  iterator_user = m_mapUser.begin();
	CUser* pUser = nullptr;
	CUser* pEnemy = nullptr;
	for (; iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);
		if (sender->GetUserID() != pUser->GetUserID())
			pEnemy = pUser;
	}
	pEnemy->OnSendGamePlayBoom(posInfo);

	return TRUE;
}

BOOL CRoom::OnSendGamePlayEnemySuperGuard(CUser * sender)
{
	MAP_USER::iterator  iterator_user = m_mapUser.begin();
	CUser* pUser = nullptr;
	CUser* pEnemy = nullptr;
	for (; iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);
		if (sender->GetUserID() != pUser->GetUserID())
			pEnemy = pUser;
	}
	pEnemy->OnSendGamePlayEnemySuperGuard();

	return TRUE;
}

BOOL CRoom::OnSendGamePlayRestart(CUser * sender)
{
	MAP_USER::iterator  iterator_user = m_mapUser.begin();
	CUser* pUser = nullptr;
	CUser* pEnemy = nullptr;
	for (; iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);
		if (sender->GetUserID() != pUser->GetUserID())
			pEnemy = pUser;
	}
	pEnemy->OnSendGamePlayRestart();

	return TRUE;
}
