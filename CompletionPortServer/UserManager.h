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
	void OnEraseUser(DWORD user_id); //������ ������ ���� ���. �ʿ��� ������ ��� �����ٰ� �ٽ� �߰���.

	eLoginResut CheckUser(string id, string pwd, CUser *pUser);
	CUser *FindUser(DWORD user_id);
	CUser *FindUser(std::string str_id);
	CUser *GetListReadyUser();

	CUser *CheckConnect(std::string str_id, char mode, DWORD userid, CUser *pUser);

	//---------------------------------- ��Ŷ ó�� ���� ----------------------------//
	void OnSendAllUser(WSABUF *pWsaBuff);
	void OnSendAllUser(); //��ü �������� ������.
	void OnSendAllUserExceptMe(CUser *pMe);// ���� ������ ��� �������� ������.
	void OnSendSeeAllUser(CUser *pActor, int me_world, int me_map); //�þ߿� ���̴� ��� �������� ������.
	void OnSendSeeAllUserExceptMe(CUser *pMe, int me_world, int me_map); // ���� ������ �þ߿� ���̴� �������� ������.
	void OnSendSeeAllUserInfoToMe(CUser *pMe, int me_world, int me_map);// �ٸ� ��� ������ ������ ������.

    void OnSendLobbyUserInfoToMe(CUser *pMe);// �ٸ� ��� ������ ������ ������.lobby
	void OnSendLobbyRoomInfoToMe(CUser *pMe);// �ٸ� ����� ���� �� ������ ������ ������.

	//�����߰�/////////////////////////////////////////////////////
	void OnSendNotify(char *notify);	//��ü ���� �޽��� ����
	void OnCheckPing();					//30�ʿ� �ѹ��� PKT_PING ��Ŷ�� ������
	///////////////////////////////////////////////////////////////
};

#endif
