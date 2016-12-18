//////////////////////////////////////////////////////////////////////
// UserManager.h
//////////////////////////////////////////////////////////////////////

#ifndef _ROOMMANAGER_H_
#define _ROOMMANAGER_H_

#include "Room.h"
#include "array_list_container.h"

#include <string>

using namespace std;

class CUser;
class CRoomManager
{
public:
	static CRoomManager *m_pRoomManager;

protected:
	CArrayListContainer<CRoom> *m_queRoomContainer;

	MAP_ROOM            m_mapRoom;
	DWORD				m_iRoomIndex;	//방번호
	
	CRITICAL_SECTION       m_CriticalSection;

protected:
	void  Lock() { EnterCriticalSection(&m_CriticalSection); } 
	void  UnLock() { LeaveCriticalSection(&m_CriticalSection); }


public:
	CRoomManager();
	virtual ~CRoomManager();

	void InitRoomManager();
	void ReleaseRoomManager();

	CRoom *CreateRoom(char *title, char* roomMasterName);
					//클라이언트로 받는제묙!
	void OnAddRoom(DWORD Room_id, CRoom *pRoom);
	void OnDeleteRoom(DWORD Room_id, CRoom *pRoom);
	void OnEraseRoom(DWORD Room_id);

	CRoom* GetRoomByNumber(DWORD roomNum);

	CRoom *GetListReadyRoom();

	//---------------------------------- 패킷 처리 관련 ----------------------------//
	void OnSendAllUser(WSABUF *pWsaBuff);
	void OnSendAllUser(); //전체 유저한테 보낸다.
	//void OnSendAllUserExceptMe(CRoom *pMe);// 나를 제외한 모든 유저한테 보낸다.
	void OnSendSeeAllUser(CRoom *pActor, int me_world, int me_map); //시야에 보이는 모든 유저한테 보낸다.
	void OnSendSeeAllUserExceptMe(CRoom *pMe, int me_world, int me_map); // 나를 제외한 시야에 보이는 유저한테 보낸다.
	void OnSendSeeAllUserInfoToMe(CRoom *pMe, int me_world, int me_map);// 다른 사람 정보를 나한테 보낸다.
	
};

#endif
