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
	DWORD				m_iRoomIndex;	//���ȣ
	
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
					//Ŭ���̾�Ʈ�� �޴�����!
	void OnAddRoom(DWORD Room_id, CRoom *pRoom);
	void OnDeleteRoom(DWORD Room_id, CRoom *pRoom);
	void OnEraseRoom(DWORD Room_id);

	CRoom* GetRoomByNumber(DWORD roomNum);

	CRoom *GetListReadyRoom();

	//---------------------------------- ��Ŷ ó�� ���� ----------------------------//
	void OnSendAllUser(WSABUF *pWsaBuff);
	void OnSendAllUser(); //��ü �������� ������.
	//void OnSendAllUserExceptMe(CRoom *pMe);// ���� ������ ��� �������� ������.
	void OnSendSeeAllUser(CRoom *pActor, int me_world, int me_map); //�þ߿� ���̴� ��� �������� ������.
	void OnSendSeeAllUserExceptMe(CRoom *pMe, int me_world, int me_map); // ���� ������ �þ߿� ���̴� �������� ������.
	void OnSendSeeAllUserInfoToMe(CRoom *pMe, int me_world, int me_map);// �ٸ� ��� ������ ������ ������.
	
};

#endif
