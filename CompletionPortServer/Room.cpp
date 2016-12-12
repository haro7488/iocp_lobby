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
int CRoom::OnAddUser(DWORD user_id, CUser *pUser)		// 방에 유저 들어옴
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
void CRoom::OnDeleteAddUser(DWORD user_id, CUser *pUser)	// 유저 나감
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
		//OutputDebugString("없는 유저를 지운다...\n");
	}
#endif	

}
//--------------------------------- 패킷 송신 관련 ---------------------------------//
BOOL CRoom::OnSendRoomInfo()
{
    ST_ROOM_INFO *pRoomInfo = (ST_ROOM_INFO *)m_SendBuff;
    pRoomInfo->PktID = PKT_ROOMINFO;
    pRoomInfo->PktSize = sizeof(ST_ROOM_INFO);
    pRoomInfo->max = m_maxUser;
    pRoomInfo->cur = m_curUser;
    pRoomInfo->index = m_iRoom;
    strcpy(pRoomInfo->title, m_strRoomName.c_str());

    m_WSABUF.len = pRoomInfo->PktSize;

    g_pUserMgr->OnSendAllUser(&m_WSABUF);

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
