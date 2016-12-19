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

void CUser::QuitRoom()
{
	if (m_myRoom == nullptr)
		return;

	m_myRoom->OnSendEndGame();

	int ret = strcmp(m_myRoom->GetRoomMasterName().c_str(), m_strUserID);
	if (ret == 0)
	{
		m_myRoom->OnDeleteAddUser(m_iUserID, this);
		m_myRoom->QuitAllUser();
		g_pRoomMgr->OnDeleteRoom(m_myRoom->GetRoomNumber(), m_myRoom);		
	}
	else
		m_myRoom->OnDeleteAddUser(m_iUserID, this);
	ST_EXIT_ROOM_RES *pExitRes = (ST_EXIT_ROOM_RES*)m_SendBuff;
	pExitRes->PktID = PKT_EXITROOMRES;
	pExitRes->PktSize = sizeof(ST_EXIT_ROOM_RES);
	OnSendPacket();

	m_myRoom = nullptr;
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
			g_pUserMgr->OnSendLobbyRoomInfoToMe(this);
			ST_LOBBY_END_OF_ROOM_INFO* pEndOfUserInfo = (ST_LOBBY_END_OF_ROOM_INFO*)m_SendBuff;
			pEndOfUserInfo->PktID = PKT_LOBBYENDOFROOMINFO;
			pEndOfUserInfo->PktSize = sizeof(ST_LOBBY_END_OF_ROOM_INFO);
			OnSendPacket();
			g_pUserMgr->OnSendLobbyUserInfoToMe(this);
		}
        break;
        case PKT_CREATEROOMREQ:
        {
            ST_ROOM_CREATE_REQ *pRoomReq = (ST_ROOM_CREATE_REQ *)pPacket;
            
            m_myRoom = g_pRoomMgr->CreateRoom(pRoomReq->title, m_strUserID);

			ST_ROOM_CREATE_RES *pRoomRes = (ST_ROOM_CREATE_RES*)m_SendBuff;
			pRoomRes->PktID = PKT_CREATEROOMRES;
			pRoomRes->PktSize = sizeof(ST_ROOM_CREATE_RES);
			pRoomRes->roomNum = m_myRoom->GetRoomNumber();
			OnSendPacket();
            //m_myRoom->OnSendAllUserRoomInfo();
        }
        break;
		case PKT_ENTERROOMREQ:
		{
			ST_ENTER_ROOM_REQ *pEnterReq = (ST_ENTER_ROOM_REQ*)pPacket;

			m_myRoom = g_pRoomMgr->GetRoomByNumber(pEnterReq->roomNum);
			eEnterRoomResult result;
			if (m_myRoom == nullptr)
			{
				result = EnterRoomFailByNoRoom;
			}
			else
			{
				result = m_myRoom->OnAddUser(m_iUserID, this);
			}

			ST_ENTER_ROOM_RES *pEnterRes = (ST_ENTER_ROOM_RES*)m_SendBuff;
			pEnterRes->PktID = PKT_ENTERROOMRES;
			pEnterRes->PktSize = sizeof(ST_ENTER_ROOM_RES);
			pEnterRes->result = result;
			OnSendPacket();

		}
		break;
		case PKT_EXITROOMREQ:
		{
			QuitRoom();
		}
		break;
		case PKT_ROOMINFOREQ:
		{
			if(m_myRoom != nullptr)
				m_myRoom->OnSendInRoomInfo(this);
		}
		break;
		case PKT_STARTGAMEREQ:
		{
			if (m_myRoom->GetCurUserCnt() == 2)
			{
				m_myRoom->OnSendStartGame();
			}
		}
		break;
		
		////GamePlay Process
		case PKT_MOVE:
		{
			//POSITIONINFO* posInfo = (POSITIONINFO*)pHeader;
			//player->SetPosition(posInfo->x, posInfo->y);
			m_myRoom->OnSendGamePlayMove(this, (POSITIONINFO*)pPacket);
		}
		break;
		case PKT_SHOOT:
		{
			//POSITIONINFO* posInfo = (POSITIONINFO*)pHeader;
			//player->AddBullet(posInfo->x, posInfo->y);
			m_myRoom->OnSendGamePlayShoot(this, (POSITIONINFO*)pPacket);
		}
		break;
		case PKT_BOOM:
		{
			//POSITIONINFO* posInfo = (POSITIONINFO*)pHeader;
			//player->AddBoom(posInfo->x, posInfo->y);
			m_myRoom->OnSendGamePlayBoom(this, (POSITIONINFO*)pPacket);
		}
		break;
		case PKT_ENEMYSUPERGUARD:
		{
			//player->BeginSuperGuard();
			m_myRoom->OnSendGamePlayEnemySuperGuard(this);
		}
		break;
		case PKT_RESTART:
		{
			//Init();
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

BOOL CUser::OnSendStartGame()
{
	ST_START_GAME_RES *pStartGameRes = (ST_START_GAME_RES*)m_SendBuff;
	pStartGameRes->PktID = PKT_STARTGAMERES;
	pStartGameRes->PktSize = sizeof(ST_START_GAME_RES);
	OnSendPacket();
	return TRUE;
}

BOOL CUser::OnSendEndGame()
{
	ST_END_GAME_RES *pEndGameRes = (ST_END_GAME_RES*)m_SendBuff;
	pEndGameRes->PktID = PKT_ENDGAMERES;
	pEndGameRes->PktSize = sizeof(ST_END_GAME_RES);
	OnSendPacket();
	return TRUE;
}

BOOL CUser::OnSendGamePlayMove(POSITIONINFO* posInfo)
{
	POSITIONINFO* pMove = (POSITIONINFO*)m_SendBuff;
	pMove->PktID = PKT_MOVE;
	pMove->PktSize = sizeof(POSITIONINFO);
	pMove->x = posInfo->x;
	pMove->y = posInfo->y;
	OnSendPacket();
	return TRUE;
}

BOOL CUser::OnSendGamePlayShoot(POSITIONINFO* posInfo)
{
	POSITIONINFO* pShot = (POSITIONINFO*)m_SendBuff;
	pShot->PktID = PKT_SHOOT;
	pShot->PktSize = sizeof(POSITIONINFO);
	pShot->x = posInfo->x;
	pShot->y = posInfo->y;
	OnSendPacket();
	return TRUE;
}

BOOL CUser::OnSendGamePlayBoom(POSITIONINFO* posInfo)
{
	POSITIONINFO* pBoom = (POSITIONINFO*)m_SendBuff;
	pBoom->PktID = PKT_BOOM;
	pBoom->PktSize = sizeof(POSITIONINFO);
	pBoom->x = posInfo->x;
	pBoom->y = posInfo->y;
	OnSendPacket();
	return TRUE;
}

BOOL CUser::OnSendGamePlayEnemySuperGuard()
{
	PACKETHEADER* pSuperGuard = (PACKETHEADER*)m_SendBuff;
	pSuperGuard->PktID = PKT_ENEMYSUPERGUARD;
	pSuperGuard->PktSize = sizeof(PACKETHEADER);
	OnSendPacket();
	return TRUE;
}

BOOL CUser::OnSendGamePlayRestart()
{
	PACKETHEADER* pRestart = (PACKETHEADER*)m_SendBuff;
	pRestart->PktID = PKT_RESTART;
	pRestart->PktSize = sizeof(PACKETHEADER);
	OnSendPacket();
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

