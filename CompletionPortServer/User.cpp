//======================================================================
//user.cpp


#include "User.h"
#include "UserManager.h"
#include "RoomManager.h"





#define WSA_IO_PENDING_MAX_ERROR    5

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUser::CUser()
{
	m_strUserID[0] = '\0';

	m_recvBuffer.len = RECV_BUFF_SIZE;
	m_recvBuffer.buf = m_packetBuffer;

	m_pIORecvOverlapped = new PER_IO_OVERLAPPED;
	m_pIOSendOverlapped = new PER_IO_OVERLAPPED;
	
	InitializeCriticalSection(&m_CriticalSection);
}

CUser::~CUser()
{
	SAFE_DELETE(m_pIORecvOverlapped);
	SAFE_DELETE(m_pIOSendOverlapped);

	DeleteCriticalSection(&m_CriticalSection);
}

void CUser::SetUserStringID(std::string id)
{ 
    strcpy(m_strUserID, id.c_str()); 
}


BOOL CUser::InitUser(DWORD user_id, SOCKET client_socket, SOCKADDR_IN &clientaddr)
{
	memset(m_pIORecvOverlapped, 0, sizeof(PER_IO_OVERLAPPED));
	memset(m_pIOSendOverlapped, 0, sizeof(PER_IO_OVERLAPPED));

	m_pIORecvOverlapped->io_type = IO_RECV;
	m_pIOSendOverlapped->io_type = IO_SEND;

	m_packetQueue.InitZeroQueue();

	ReleaseUser();

	m_hSocket = client_socket;

	m_iUserID = user_id;
	m_strUserID[0] = '\0';
    m_myRoom = nullptr;


	if( !RegisterCompletionPort(m_hSocket, (DWORD)this) )
	{
		return FALSE;
	}

	OnInitUserSocket();

	return TRUE;
}

void CUser::ReleaseSocketContext()
{
	memset(m_pIORecvOverlapped, 0, sizeof(PER_IO_OVERLAPPED));
	memset(m_pIOSendOverlapped, 0, sizeof(PER_IO_OVERLAPPED));
}

void CUser::ReleaseUser()
{

}

//--------------------------------------------------------------------------------
//
void CUser::OnInitUserSocket()
{
	if( m_hSocket == INVALID_SOCKET ) return;
	if( m_pIORecvOverlapped->io_type == IO_NONE ) return; //이미 끊긴 유저다

	int      nRet;
	DWORD    dwRecvNumBytes = 0;
	DWORD    dwFlags = 0;	
	
	memset(m_pIORecvOverlapped, 0, sizeof(PER_IO_OVERLAPPED)); m_pIORecvOverlapped->io_type = IO_RECV;

	nRet = WSARecv( m_hSocket, &m_recvBuffer, 1,
					&dwRecvNumBytes, &dwFlags, (WSAOVERLAPPED *)m_pIORecvOverlapped, NULL );
	if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) 
	{			
		//OutputDebugString("WSARecv Failed!.....................................\n");
		CloseUserSocket();
	}
}

// 서버가 접속한 유저의 소켓을 닫을때.
// 완료 포트한테 해당 유저의 소켓을 닫으라구 알려준다.
void CUser::CloseUserSocket(BOOL isPost)
{	
	if(m_hSocket == INVALID_SOCKET) return;
	
	m_pIORecvOverlapped->io_type = IO_NONE;
	shutdown(m_hSocket, SD_RECEIVE); //받기금지	

	if(isPost)
	{
		BOOL ret = NotifyCompletionStatus((DWORD)this, 0, (WSAOVERLAPPED *)m_pIORecvOverlapped, NULL);
		if (ret == FALSE)
		{
			//OutputDebugString("[ERROR]소켓 닫기 메시지 전송 실패[CUser::CloseUserSocket]\n");
		}
	}
	closesocket(m_hSocket);
}


void CUser::OnDisconnectSocket()
{

}


//--------------------------------- 패킷 처리 -----------------------------------//
//--------------------------------------------------------------------------------
//message thread 에서 호출한다.
void CUser::OnPacketProcess(void *pPacket)
{
	DWORD packet_id;
	WORD packet_size;
	PACKETHEADER *pRecvPacket = (PACKETHEADER *)pPacket;

	packet_id = pRecvPacket->PktID;
	packet_size = pRecvPacket->PktSize;

	switch( packet_id )
	{
		case PKT_LOGIN:
		{
			ST_LOGIN* pLogin = (ST_LOGIN*)pPacket;
			eLoginResut result = CUserManager::m_pUserManager->CheckUser(pLogin->name, pLogin->pwd, this);
			ST_LOGIN_RESULT *pLoginResult = (ST_LOGIN_RESULT *)m_SendBuff;
			pLoginResult->PktID = PKT_LOGINRESULT;
			pLoginResult->PktSize = sizeof(ST_LOGIN_RESULT);
			pLoginResult->result = result;
            

			OnSendPacket();
		}	
		break;
        case PKT_LOBBYINFOREQ:
        {
            g_pUserMgr->OnSendLobbyUserInfoToMe(this);
			g_pUserMgr->OnSendLobbyRoomInfoToMe(this);
        }
        break;
        case PKT_CREATEROOM:
        {
            ST_ROOM_CREATE *pRoomInof = (ST_ROOM_CREATE *)pPacket;
            
            m_myRoom = g_pRoomMgr->CreateRoom(pRoomInof->title);
            int ret = m_myRoom->OnAddUser(m_iUserID, this);
            if (ret != 0)
            {
                return;
            }

            m_myRoom->OnSendAllUserRoomInfo();
        }
        break;
		case PKT_ROOMINFOREQ:
		{
			m_myRoom->OnSendInRoomInfo(this);
		}
		break;
	}
}



//--------------------------------- 패킷 송신 관련 ---------------------------------//
BOOL CUser::OnSendPacket()
{
	if(m_hSocket == INVALID_SOCKET ) return FALSE;
	m_pIOSendOverlapped->io_type = IO_SEND;

	DWORD sendByte;
	PACKETHEADER *pPacket = (PACKETHEADER *)GetSendBuff();
	m_WSABUF.len = pPacket->PktSize;

	m_ret = WSASend(m_hSocket, &m_WSABUF, 1, &sendByte, 0, (WSAOVERLAPPED *)m_pIOSendOverlapped, NULL);
	if( m_ret == SOCKET_ERROR)
	{
		if( WSAGetLastError() != WSA_IO_PENDING )
		{
#ifdef _DEBUG
			m_ret = WSAGetLastError();
			//char buff[100];			
			//wsprintf(buff, "[ERROR : %d]패킷 전송 실패![CUser::OnSendPacket()]\n", m_ret);
			//OutputDebugString(buff);
#endif
			CloseUserSocket();

			return FALSE;
		}
	}

	
	//핑 체크..
	//OnCheckPing(GetTickCount());


	return TRUE;
}


//---------------------------------------------------------------------------------------------
BOOL CUser::OnSendPacket(WSABUF *pWsaBuff)
{
	if(m_hSocket == INVALID_SOCKET ) return FALSE;
	m_pIOSendOverlapped->io_type = IO_SEND;

	DWORD sendByte;
	m_ret = WSASend(m_hSocket, pWsaBuff, 1, &sendByte, 0, (WSAOVERLAPPED *)m_pIOSendOverlapped, NULL);
	if( m_ret == SOCKET_ERROR)
	{
		if( WSAGetLastError() != WSA_IO_PENDING )
		{
#ifdef _DEBUG
			m_ret = WSAGetLastError();
			//char buff[100];			
			//wsprintf(buff, "[ERROR : %d]패킷 전송 실패![CUser::OnSendPacket(WSABUF *pWsaBuff)]\n", m_ret);
			//OutputDebugString(buff);
#endif
			CloseUserSocket();

			return FALSE;
		}	
	}

	return TRUE;
}

void CUser::OnSendUserName(CUser *pUser)
{
    ST_LOBBYINFORES *pUserName = (ST_LOBBYINFORES *)m_SendBuff;
    pUserName->PktID = PKT_LOBBYINFORES;
    pUserName->PktSize = sizeof(ST_LOBBYINFORES);
    strcpy(pUserName->name, m_strUserID);

    m_WSABUF.len = pUserName->PktSize;
    pUser->OnSendPacket(&m_WSABUF);
}

