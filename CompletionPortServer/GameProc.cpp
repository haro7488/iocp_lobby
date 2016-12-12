//////////////////////////////////////////////////////////////////////
// GameProc.cpp
//////////////////////////////////////////////////////////////////////

#include "GameProc.h"
#include "UserManager.h"


//������ ������ ��ȣ.
//������ �����Ҷ� ���� 1�� �����ؼ� �ο��Ѵ�.
static DWORD g_userID = 100;


//���� ������ �ִ� ���丮 ���.
char CURRENT_DIRECTORY[MAX_PATH];

//crc ������ �ִ� ���丮 ���.
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



//---------------------------- ���� �޽��� ���� ----------------------------//



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
