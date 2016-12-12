//////////////////////////////////////////////////////////////////////
// GameProc.cpp
//////////////////////////////////////////////////////////////////////

#include "GameProc.h"
#include "UserManager.h"


//접속한 유저의 번호.
//유저가 접속할때 마다 1씩 증가해서 부여한다.
static DWORD g_userID = 100;


//실행 파일이 있는 디렉토리 경로.
char CURRENT_DIRECTORY[MAX_PATH];

//crc 파일이 있는 디렉토리 경로.
char CURRENT_CRC_DIRECTORY[MAX_PATH];




void btzStrcpy(char *pDest, int buffSize, char *pSrc)
{
	int len;
	try
	{
		len = strlen(pSrc);
	}
	catch(...)
	{
		len = buffSize;
	}	
	
	if(len >= buffSize)
		len = buffSize-1;

	memcpy(pDest, pSrc, len);
	pDest[len] = '\0';
}



//---------------------------- 게임 메시지 관련 ----------------------------//



//------------------------------------------------------------------------------------------------
//
void OnReceivePacket(DWORD msg, DWORD wParam, DWORD lParam, const char *pPacket)
{
	switch (msg)									
	{
		case GM_DISCONNECTUSER:
			{
				
			}
			 break;
		case GM_PKTRECEIVE:
			{
				CUser *pUser = (CUser *)lParam;

				if( (pUser == NULL) || (pPacket == NULL) )
					break;				

				pUser->OnPacketProcess((void *)pPacket);
			}
			 break;
		case GM_GAMETIMER:
			{

			}
			 break;
		default:
			{
				//OutputDebugString("default................\n");
			}
			 break;
	}
}
