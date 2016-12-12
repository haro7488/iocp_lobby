//////////////////////////////////////////////////////////////////////
// UserManager.h
//////////////////////////////////////////////////////////////////////

#ifndef _USERMANAGER_H_
#define _USERMANAGER_H_

#include "user.h"
#include "array_list_container.h"

#include <string>

using namespace std;

class CUserManager
{
public:
	static CUserManager *m_pUserManager;

protected:
	CArrayListContainer<CUser> *m_queUserContainer;

	MAP_USER            m_mapUser;
	MAP_USERID          m_mapUserID;

	DWORD               m_tickPing;
	map<string, string> m_userAuth;


	CRITICAL_SECTION       m_CriticalSection;

protected:
	void  Lock() { EnterCriticalSection(&m_CriticalSection); } 
	void  UnLock() { LeaveCriticalSection(&m_CriticalSection); }


public:
	CUserManager();
	virtual ~CUserManager();	

	void InitUserManager();
	void ReleaseUserManager();

	void OnAddUser(DWORD user_id, CUser *pUser);
	void OnDeleteUser(DWORD user_id, CUser *pUser);
	void OnEraseUser(DWORD user_id); //유저가 포털을 탔을 경우. 맵에서 유저를 잠시 지웠다가 다시 추가함.

	eLoginResut CheckUser(string id, string pwd, CUser *pUser);
	CUser *FindUser(DWORD user_id);
	CUser *FindUser(std::string str_id);
	CUser *GetListReadyUser();

	CUser *CheckConnect(std::string str_id, char mode, DWORD userid, CUser *pUser);

	//---------------------------------- 패킷 처리 관련 ----------------------------//
	void OnSendAllUser(WSABUF *pWsaBuff);
	void OnSendAllUser(); //전체 유저한테 보낸다.
	void OnSendAllUserExceptMe(CUser *pMe);// 나를 제외한 모든 유저한테 보낸다.
	void OnSendSeeAllUser(CUser *pActor, int me_world, int me_map); //시야에 보이는 모든 유저한테 보낸다.
	void OnSendSeeAllUserExceptMe(CUser *pMe, int me_world, int me_map); // 나를 제외한 시야에 보이는 유저한테 보낸다.
	void OnSendSeeAllUserInfoToMe(CUser *pMe, int me_world, int me_map);// 다른 사람 정보를 나한테 보낸다.

    void OnSendLobbyUserInfoToMe(CUser *pMe);// 다른 사람 정보를 나한테 보낸다.lobby
	void OnSendLobbyRoomInfoToMe(CUser *pMe);// 다른 사람이 만든 방 정보를 나한테 보낸다.

	//경이추가/////////////////////////////////////////////////////
	void OnSendNotify(char *notify);	//전체 공지 메시지 전송
	void OnCheckPing();					//30초에 한번씩 PKT_PING 패킷을 보낸다
	///////////////////////////////////////////////////////////////
};

#endif
