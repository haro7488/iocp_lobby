//////////////////////////////////////////////////////////////////////
// RoomManager.cpp
//////////////////////////////////////////////////////////////////////

#include "RoomManager.h"


#define LIST_READY_USER_SIZE  1000
#define PING_CHECK_TIME_ALL  180000 //모든 접속 유저의 핑 체크 시간

CRoomManager *CRoomManager::m_pRoomManager = NULL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRoomManager::CRoomManager()
{
	m_iRoomIndex = 100;
	m_pRoomManager = this;
	m_queRoomContainer = NULL;
	m_pRoomManager->InitRoomManager();
}

CRoomManager::~CRoomManager()
{
	ReleaseRoomManager();	
}


void CRoomManager::InitRoomManager()
{
	m_queRoomContainer = new CArrayListContainer<CRoom>(LIST_READY_USER_SIZE);

	InitializeCriticalSection(&m_CriticalSection);
}


CRoom *CRoomManager::GetListReadyRoom()
{	
	return m_queRoomContainer->pop();
}
CRoom *CRoomManager::CreateRoom(char *title, char *masterName)
{
	CRoom *pRoom = GetListReadyRoom();

	pRoom->SetTitle(title);
	pRoom->SetMaxUser(2);
    pRoom->SetCurUser(0);
	pRoom->SetRoomNumber(m_iRoomIndex);
	pRoom->SetRoomMasterName(masterName);

    OnAddRoom(m_iRoomIndex, pRoom);
	
	m_iRoomIndex++;

	return pRoom;

}
void CRoomManager::OnAddRoom(DWORD Room_id, CRoom *pRoom)
{
	Lock();
	m_mapRoom.insert(std::make_pair(Room_id, pRoom));
	UnLock();
}


//CRoom *CRoomManager::FindRoom(DWORD Room_id)
//{
//	Lock();
//
//	CRoom *pRoom = NULL;
//	MAP_ROOM::iterator  iterator_room;
//	iterator_room = m_mapRoom.find(Room_id);
//
//	if (iterator_room != m_mapRoom.end())
//	{
//		pRoom = (CRoom *)iterator_room->second;
//		UnLock();
//
//		return pUser;
//	}
//
//	UnLock();
//
//	return NULL;
//}
//
//CRoom *CRoomManager::FindUser(std::string str_id)
//{
//	Lock();
//
//	CUser *pUser = NULL;
//	MAP_USERID::iterator  iterator_user = m_mapUserID.find(str_id);
//	if( iterator_user != m_mapUserID.end() ) //이미 접속중인 아이디다.
//	{
//		pUser = iterator_user->second;
//	}
//
//	UnLock();
//
//	return pUser;
//}
//


void CRoomManager::OnDeleteRoom(DWORD Room_id, CRoom *pRoom)
{
	if(pRoom == NULL) return;

	Lock();

	
	MAP_ROOM::iterator iterator_room = m_mapRoom.find(Room_id);

	if (iterator_room != m_mapRoom.end())
	{		 
		m_mapRoom.erase(iterator_room);
	}
#ifdef _DEBUG
	else
	{
		//OutputDebugString("없는 유저를 지운다...\n");
	}
#endif	

	UnLock();

	m_queRoomContainer->push(pRoom);
}



void CRoomManager::ReleaseRoomManager()
{
	Lock();

	CUser *pUser = NULL;
	MAP_ROOM::iterator  iterator_room = m_mapRoom.begin();

	for (; iterator_room != m_mapRoom.end(); iterator_room++)
	{
		delete (iterator_room->second);
	}
	m_mapRoom.clear();

	//대기 리스트의 유저들도 삭제한다.
	delete m_queRoomContainer; m_queRoomContainer = NULL;

	UnLock();

	DeleteCriticalSection(&m_CriticalSection);
}


//------------------------------------------------------------------------------------------
//

void CRoomManager::OnEraseRoom(DWORD Room_id)
{
	Lock();

	MAP_ROOM::iterator  iterator_room;
	iterator_room = m_mapRoom.find(Room_id);

	if (iterator_room != m_mapRoom.end())
	{		 
		m_mapRoom.erase(iterator_room);
	}

	UnLock();
}

CRoom* CRoomManager::GetRoomByNumber(DWORD roomNum)
{
	CRoom* pRoom = nullptr;
	Lock();

	MAP_ROOM::iterator  iterator_room;
	iterator_room = m_mapRoom.find(roomNum);

	if (iterator_room != m_mapRoom.end())
	{
		pRoom = iterator_room->second;
	}

	UnLock();

	return pRoom;
}


//---------------------------- 패킷 처리 관련 --------------------------//

void CRoomManager::OnSendAllUser()
{
    CRoom *pRoom;

    Lock();

    MAP_ROOM::iterator  iterator_room = m_mapRoom.begin();

    for (; iterator_room != m_mapRoom.end(); iterator_room++)
    {
        pRoom = (iterator_room->second);
        if (pRoom == NULL) continue;

        pRoom->OnSendAllUserRoomInfo();
    }

    UnLock();
}

void CRoomManager::OnSendAllUser(WSABUF *pWsaBuff)
{
	CRoom *pRoom;

	Lock();

	MAP_ROOM::iterator  iterator_room = m_mapRoom.begin();

	for (; iterator_room != m_mapRoom.end(); iterator_room++)
	{
		pRoom = (iterator_room->second);
		if( pRoom == NULL ) continue;

		pRoom->OnSendPacket(pWsaBuff);
	}

	UnLock();
}

//void CRoomManager::OnSendAllUserExceptMe(CRoom *pMe)
//{
//    if (!pMe) return;
//
//	CRoom *pRoom;
//
//    Lock();
//
//    MAP_USERID::iterator  iterator_user = m_mapUserID.begin();
//
//    for (; iterator_user != m_mapUserID.end(); iterator_user++)
//    {
//        pUser = (iterator_user->second);
//
//        if (pMe->GetUserID() == pUser->GetUserID()) continue;
//
//        pMe->OnSendUserName(pUser);
//    }
//
//    UnLock();
//}

void CRoomManager::OnSendSeeAllUser(CRoom *pActor, int me_world, int me_map)
{
	if(!pActor) return;

	CRoom *pRoom;

	Lock();

	MAP_ROOM::iterator  iterator_room = m_mapRoom.begin();

	for (; iterator_room != m_mapRoom.end(); iterator_room++)
	{
		pRoom = (iterator_room->second);

		pRoom->OnSendPacket(pActor->GetSendWsaBuf());
	}

	UnLock();
}

void CRoomManager::OnSendSeeAllUserExceptMe(CRoom *pMe, int me_world, int me_map)
{
	if(!pMe) return;

	CRoom *pRoom;

	Lock();

	MAP_ROOM::iterator  iterator_room = m_mapRoom.begin();

	for (; iterator_room != m_mapRoom.end(); iterator_room++)
	{
		pRoom = (iterator_room->second);

		//if(pMe->GetUserID() == pUser->GetUserID()) continue;

		pRoom->OnSendPacket(pMe->GetSendWsaBuf());
	}

	UnLock();
}

void CRoomManager::OnSendSeeAllUserInfoToMe(CRoom *pMe, int me_world, int me_map)
{
	if(!pMe) return;

	CRoom *pRoom;

	Lock();

	MAP_ROOM::iterator  iterator_room = m_mapRoom.begin();

	for (; iterator_room != m_mapRoom.end(); iterator_room++)
	{
		pRoom = (iterator_room->second);

		//if (pMe->GetUserID() == pRoom->GetUserID()) continue;

//		pUser->OnSendMyInfo(pMe);
	}

	UnLock();
}
