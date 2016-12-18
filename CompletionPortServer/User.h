//////////////////////////////////////////////////////////////////////
// User.h
//////////////////////////////////////////////////////////////////////

#ifndef _USER_H_
#define _USER_H_

#pragma warning(disable : 4786)
#include <map>
#include "CirQueue.h"
//#include "GameMsgManager.h"
#include <string>
#include "SendPacket.h"
#include "GameMsgManager.h"

#define PING_CHECK_TIME  20000 //�� üũ �ð�
#define RECV_BUFF_SIZE    10000

struct BTZStringLess : std::binary_function< const std::string&, const std::string&, bool > 
{
	public:
	  bool operator()(const std::string& a1, const std::string& a2) const
	{
		   return _stricmp(a1.c_str(),a2.c_str()) < 0;
	}
};


class CUser;

typedef std::map<DWORD , CUser *> MAP_USER;
typedef std::map<std::string , CUser *, BTZStringLess>  MAP_USERID;

#define MAX_USERSTRID  30

class CRoom;
class CUser : public CSendPacket  
{
protected:	
	char            m_strUserID[MAX_USERSTRID];
	DWORD           m_iUserID;		// ���������θ� ����ϴ� ���� ���̵�

    CRoom           *m_myRoom;


	PPER_IO_OVERLAPPED     m_pIORecvOverlapped;	
	PPER_IO_OVERLAPPED     m_pIOSendOverlapped;	

	CCirQueue              m_packetQueue;
	WSABUF                 m_recvBuffer;
	char                   m_packetBuffer[RECV_BUFF_SIZE];
	int                    m_RecvSize;

	CRITICAL_SECTION       m_CriticalSection; 

	BOOL                   m_isLogin;


protected:
	void  Lock() { EnterCriticalSection(&m_CriticalSection); } 
	void  UnLock() { LeaveCriticalSection(&m_CriticalSection); }


public:
	CUser();
	virtual ~CUser();
	
	DWORD GetUserID() const { return m_iUserID; }
	char  *GetUserStringID() { return m_strUserID; }
    void SetUserStringID(std::string id);
	
	CRoom* GetRoom() { return m_myRoom; }
	void QuitRoom();

	void OnInitUserSocket();
	void OnDisconnectSocket();
	void CloseUserSocket(BOOL isPost = FALSE);

	PPER_IO_OVERLAPPED  GetOverlapped() { return m_pIORecvOverlapped; }
	int GetSendOverlappedState() { return (m_pIOSendOverlapped == NULL) ? IO_SEND : m_pIOSendOverlapped->io_type; }
	void ReleaseSocketContext();


	//--------------------------------- �ʱ�ȭ ���� ---------------------------------//
	BOOL InitUser(DWORD user_id, SOCKET client_socket, SOCKADDR_IN &clientaddr);
	void ReleaseUser();


	//--------------------------------- ��Ŷ �۽� ���� ---------------------------------//
	BOOL   OnSendPacket(); //�ڽ��� ������ ���� ������ ��Ŷ...
	BOOL   OnSendPacket(WSABUF *pWsaBuff); // �ٸ� ������ ������ ������.

    void OnSendUserName(CUser *pUser);

	//------------------------- ��Ŷ ó�� ���� -----------------------//
	inline BOOL OnSendGamePacket(int recv_byte);

	void OnPacketProcess(void *pPacket); //message thread ���� ȣ���Ѵ�.										



};



inline BOOL CUser::OnSendGamePacket(int recv_byte)
{
	Lock();


	int recvsize = m_packetQueue.OnPutData(m_recvBuffer.buf, recv_byte);
	if(recvsize == -1)
	{
		UnLock();
		return FALSE;
	}


	PACKETHEADER *pPacket;

	while( 1 )
	{
		if( (pPacket = m_packetQueue.GetPacket()) == NULL )
		{
			//UnLock();
			break;		
		}

		g_pGameMgr->SendGameMessage(GM_PKTRECEIVE, (DWORD)(pPacket->PktSize), (DWORD)(this), (char *)pPacket);

		//OnPacketProcess(pPacket);

		//memcpy(m_SendBuff, pPacket, pPacket->PktSize);
		//OnSendPacket();

		m_packetQueue.OnPopData(pPacket->PktSize);	
	}

	UnLock();

	OnInitUserSocket();

	return TRUE;
}


#endif
