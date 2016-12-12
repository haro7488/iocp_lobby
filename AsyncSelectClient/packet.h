////////////////////////////////////////////////////////////
// packet.h
////////////////////////////////////////////////////////////

#ifndef _PACKET_H_
#define _PACKET_H_

#include <windows.h>

#pragma pack(push)
#pragma pack(1)

typedef struct _tgPacketHeader
{
	DWORD	PktID;
	WORD	PktSize;
}PACKETHEADER;

#define PKT_MOUSPOSITION	0x00100011
typedef struct _tgMousePosition : PACKETHEADER
{
	int x;
	int y;
}MOUSEPOSITION;

#define PKT_LOGIN	0xa0000001
typedef struct _tgLogin : public PACKETHEADER
{
	char name[30];
	char pwd[30];
} ST_LOGIN;

enum eLoginResut
{
    loginSuccess = 0,
    loginFailWithNoUser = 1,
    loginFailWithWorngPassward = 2,
    loginFailWithExistUser = 3
};

#define PKT_LOGINRESULT		0x00110022
typedef struct _tgLoginResult : PACKETHEADER
{
	eLoginResut result;
} ST_LOGIN_RESULT;

#define PKT_LOBBYINFOREQ    0xff000011
typedef struct _tgLobbyInfoReq : PACKETHEADER
{                 
} ST_LOBBYINFOREQ;

#define PKT_LOBBYINFORES    0xff000012
typedef struct _tgLobbyInfoRes : PACKETHEADER
{
    char name[30];
} ST_LOBBYINFORES;

#define PKT_CREATEROOM		0x00220011
typedef struct _tgCreateRoom : PACKETHEADER
{
	char title[50];
} ST_ROOM_CREATE;
#define PKT_ROOMINFO		0x00220012
typedef struct _tgRoomInfo : PACKETHEADER
{
	DWORD index;
	unsigned char max;
	unsigned char cur;
	char title[50];
}ST_ROOM_INFO;
#pragma pack(pop)

#endif
