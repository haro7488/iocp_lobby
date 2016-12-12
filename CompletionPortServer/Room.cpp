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
int CRoom::OnAddUser(DWORD user_id, CUser *pUser)		// �濡 ���� ����
{
    if (m_curUser >= m_maxUser)
        return 1;

	if (FindUser(user_id))
	{
		return 3;
	}

    m_curUser++;

	m_mapUser.insert(std::make_pair(user_id, pUser));

    return 0;
}
void CRoom::OnDeleteAddUser(DWORD user_id, CUser *pUser)	// ���� ����
{
	if (pUser == NULL) return;



	MAP_USER::iterator iterator_user = m_mapUser.find(user_id);

	if (iterator_user != m_mapUser.end())
	{
		m_mapUser.erase(iterator_user);

	}
#ifdef _DEBUG
	else
	{
		//OutputDebugString("���� ������ �����...\n");
	}
#endif	

}
//--------------------------------- ��Ŷ �۽� ���� ---------------------------------//
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
