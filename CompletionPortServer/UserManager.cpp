//////////////////////////////////////////////////////////////////////
// UserManager.cpp
//////////////////////////////////////////////////////////////////////

#include "UserManager.h"
#include "Room.h"


#define LIST_READY_USER_SIZE  1000
#define PING_CHECK_TIME_ALL  180000 //��� ���� ������ �� üũ �ð�

CUserManager *CUserManager::m_pUserManager = NULL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserManager::CUserManager()
{
	m_pUserManager = this;
	m_queUserContainer = NULL;
	m_pUserManager->InitUserManager();
}

CUserManager::~CUserManager()
{
	ReleaseUserManager();	
}


void CUserManager::InitUserManager()
{
	m_queUserContainer = new CArrayListContainer<CUser>(LIST_READY_USER_SIZE);

	InitializeCriticalSection(&m_CriticalSection);
	m_tickPing = ::GetTickCount();

	FILE *fp = fopen("user_auth.txt", "rt");
	while (!feof(fp))
	{
		char id[30], pwd[30];
		fscanf(fp, "%s %s", id, pwd);

		string s_id = id,
			s_pwd = pwd;
		m_userAuth[s_id] = s_pwd;
	}

	fclose(fp);
}


CUser *CUserManager::GetListReadyUser()
{	
	return m_queUserContainer->pop();
}

void CUserManager::OnAddUser(DWORD user_id, CUser *pUser)
{
	if(FindUser(user_id))
	{
		return;
	}

	Lock();
	m_mapUser.insert(std::make_pair(user_id, pUser));
	UnLock();
}

eLoginResut CUserManager::CheckUser(string id, string pwd, CUser *pUser)
{
    if (FindUser(id) != NULL)
        return loginFailWithExistUser;

	auto it = m_userAuth.find(id);
	if (it == m_userAuth.end())
		return loginFailWithNoUser;
	if (pwd != m_userAuth[id])
		return loginFailWithWorngPassward;

    pUser->SetUserStringID(id);

    Lock();
    m_mapUserID[pUser->GetUserStringID()] = pUser;
    UnLock();

    OnSendAllUserExceptMe(pUser);

	return loginSuccess;
}

CUser *CUserManager::FindUser(DWORD user_id)
{
	Lock();

	CUser *pUser = NULL;
	MAP_USER::iterator  iterator_user;
	iterator_user = m_mapUser.find(user_id);

	if(iterator_user != m_mapUser.end())
	{
		pUser = (CUser *)iterator_user->second;
		UnLock();

		return pUser;
	}

	UnLock();

	return NULL;
}

CUser *CUserManager::FindUser(std::string str_id)
{
	Lock();

	CUser *pUser = NULL;
	MAP_USERID::iterator  iterator_user = m_mapUserID.find(str_id);
	if( iterator_user != m_mapUserID.end() ) //�̹� �������� ���̵��.
	{
		pUser = iterator_user->second;
	}

	UnLock();

	return pUser;
}



void CUserManager::OnDeleteUser(DWORD user_id, CUser *pUser)
{
	if(pUser == NULL) return;

	Lock();

	
	MAP_USER::iterator iterator_user = m_mapUser.find(user_id);	

	if( iterator_user != m_mapUser.end() )
	{		 
		m_mapUser.erase(iterator_user);	

		MAP_USERID::iterator it = m_mapUserID.find(pUser->GetUserStringID());
		if(it != m_mapUserID.end())
			m_mapUserID.erase(it);
	}
#ifdef _DEBUG
	else
	{
		//OutputDebugString("���� ������ �����...\n");
	}
#endif	

	UnLock();

	m_queUserContainer->push(pUser);
}



void CUserManager::ReleaseUserManager()
{
	Lock();

	CUser *pUser = NULL;
	MAP_USER::iterator  iterator_user = m_mapUser.begin();

	for( ;iterator_user != m_mapUser.end(); iterator_user++)
	{
		delete (iterator_user->second);
	}
	m_mapUser.clear();
	m_mapUserID.clear();

	//��� ����Ʈ�� �����鵵 �����Ѵ�.
	delete m_queUserContainer; m_queUserContainer = NULL;

	UnLock();

	DeleteCriticalSection(&m_CriticalSection);
}


//------------------------------------------------------------------------------------------
//
CUser *CUserManager::CheckConnect(std::string str_id, char mode,  DWORD userid, CUser *pUser)
{
	Lock();

	CUser *pConUser = NULL;
	MAP_USER::iterator    it;
	MAP_USERID::iterator  iterator_user = m_mapUserID.find(str_id);
	if( iterator_user != m_mapUserID.end() ) //�̹� �������� ���̵��.
	{
		pConUser = iterator_user->second;

		//������ ���� ���� ���...
		if(mode != 0)
		{
			m_mapUserID.erase(iterator_user);	
			m_mapUserID.insert(std::make_pair(str_id, pUser));

			it = m_mapUser.find(pConUser->GetUserID());
			if(it != m_mapUser.end())
				m_mapUser.erase(it);				

			m_mapUser.insert(std::make_pair(userid, pUser));
		}

		UnLock();
		return pConUser;
	}

	//���� ������ ������ ���...
	m_mapUserID.insert(std::make_pair(str_id, pUser));
	m_mapUser.insert(std::make_pair(userid, pUser));

	UnLock();

	return NULL;
}

void CUserManager::OnEraseUser(DWORD user_id)
{
	Lock();

	MAP_USER::iterator  iterator_user;
	iterator_user = m_mapUser.find(user_id);

	if(iterator_user != m_mapUser.end())
	{		 
   	    m_mapUser.erase(iterator_user);
	}

	UnLock();
}


//---------------------------- ��Ŷ ó�� ���� --------------------------//

void CUserManager::OnSendAllUser()
{

}

void CUserManager::OnSendAllUser(WSABUF *pWsaBuff)
{
	CUser *pUser;

	Lock();

	MAP_USER::iterator  iterator_user = m_mapUser.begin();

	for( ;iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);
		if( pUser == NULL ) continue;

		pUser->OnSendPacket(pWsaBuff);
	}

	UnLock();
}

void CUserManager::OnSendAllUserExceptMe(CUser *pMe)
{
    if (!pMe) return;

    CUser *pUser;

    Lock();

    MAP_USERID::iterator  iterator_user = m_mapUserID.begin();

    for (; iterator_user != m_mapUserID.end(); iterator_user++)
    {
        pUser = (iterator_user->second);

        if (pMe->GetUserID() == pUser->GetUserID()) continue;

        pMe->OnSendUserName(pUser);
    }

    UnLock();
}

void CUserManager::OnSendSeeAllUser(CUser *pActor, int me_world, int me_map)
{
	if(!pActor) return;

	CUser *pUser;

	Lock();

	MAP_USER::iterator  iterator_user = m_mapUser.begin();

	for( ;iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);

		pUser->OnSendPacket(pActor->GetSendWsaBuf());
	}

	UnLock();
}

void CUserManager::OnSendSeeAllUserExceptMe(CUser *pMe, int me_world, int me_map)
{
	if(!pMe) return;

	CUser *pUser;

	Lock();

	MAP_USER::iterator  iterator_user = m_mapUser.begin();

	for( ;iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);

		if(pMe->GetUserID() == pUser->GetUserID()) continue;

		pUser->OnSendPacket(pMe->GetSendWsaBuf());
	}

	UnLock();
}

void CUserManager::OnSendSeeAllUserInfoToMe(CUser *pMe, int me_world, int me_map)
{
	if(!pMe) return;

	CUser *pUser;

	Lock();

	MAP_USER::iterator  iterator_user = m_mapUser.begin();

	for( ;iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);

		if(pMe->GetUserID() == pUser->GetUserID()) continue;

//		pUser->OnSendMyInfo(pMe);
	}

	UnLock();
}

void CUserManager::OnSendLobbyUserInfoToMe(CUser *pMe)
{
    if (!pMe) return;

    CUser *pUser;

    Lock();

    MAP_USERID::iterator  iterator_user = m_mapUserID.begin();

    for (; iterator_user != m_mapUserID.end(); iterator_user++)
    {
        pUser = (iterator_user->second);

        if (pMe->GetUserID() == pUser->GetUserID()) continue;

		if (pUser->GetRoom() != nullptr) continue;

        pUser->OnSendUserName(pMe);
    }

    UnLock();
}

void CUserManager::OnSendLobbyRoomInfoToMe(CUser * pMe)
{
	if (!pMe) return;

	CUser *pUser;

	Lock();

	MAP_USERID::iterator  iterator_user = m_mapUserID.begin();

	for (; iterator_user != m_mapUserID.end(); iterator_user++)
	{
		pUser = (iterator_user->second);

		if (pMe->GetUserID() == pUser->GetUserID()) continue;

		CRoom* room = pUser->GetRoom();
		if (!room) continue;
		//pUser->OnSendUserName(pMe);
		room->OnSendRoomInfo(pMe);
	}

	UnLock();
}


//�����߰�( ��ü ���� �޽��� )
void CUserManager::OnSendNotify(char *notify)
{
	CUser *pUser;

	Lock();

	MAP_USER::iterator iterator_user = m_mapUser.begin();
	for( ;iterator_user != m_mapUser.end(); iterator_user++)
	{
		pUser = (iterator_user->second);
		if( pUser == NULL ) continue;
	}

	UnLock();
}

//�����߰�(30�ʿ� �ѹ��� PKT_PING ��Ŷ�� ������)
void CUserManager::OnCheckPing()
{
	DWORD tick = ::GetTickCount();
	if(tick - m_tickPing < PING_CHECK_TIME_ALL)
	   return;

	m_tickPing = tick;

	CUser *pUser;

	Lock();

	MAP_USER::iterator iterator_user = m_mapUser.begin();
	for( ;iterator_user != m_mapUser.end(); iterator_user++ )
	{
		pUser = (iterator_user->second);
		if( pUser == NULL ) continue;	

	}

	UnLock();
}

