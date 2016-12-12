#pragma once

#pragma warning(disable : 4786)
#include <map>
#include "CirQueue.h"
//#include "GameMsgManager.h"
#include <string>
#include "SendPacket.h"
#include "User.h"

class CRoom;
typedef std::map<DWORD, CRoom *> MAP_ROOM;

class CRoom : public CSendPacket
{
protected:
	DWORD			m_iRoom;		//���ȣ
	unsigned char	m_curUser;		//�ִ��ο��� 255�̻����̾���� unsigned
	unsigned char	m_maxUser;
	std::string			m_strRoomName;
	
	MAP_USER m_mapUser;				//�濡����� �����ִ��� �˱����

public:
	CRoom();
	virtual ~CRoom();

	CUser *FindUser(DWORD user_id);
	
	void SetTitle(char * title){ m_strRoomName = title; }
	void SetMaxUser(unsigned char max){ m_maxUser = max; }
	void SetCurUser(unsigned char cur){ m_curUser = cur; }
	void SetRoomNumber(DWORD number){ m_iRoom = number; }

	int OnAddUser(DWORD user_id, CUser *pUser);		// �濡 ���� ����
	void OnDeleteAddUser(DWORD user_id, CUser *pUser);	// ���� ����

	//----------------��Ŷ �۽� ����------------------ -
    BOOL OnSendRoomInfo();
	BOOL OnSendPacket();
	BOOL OnSendPacket(WSABUF *pWsaBuff);
};
