////////////////////////////////////////////////////////////
// packet.h
////////////////////////////////////////////////////////////

#ifndef _PACKET_H_
#define _PACKET_H_

#include "global.h"

#pragma pack(push)
#pragma pack(1)

#define PKTHEADERSIZE          6 // 패킷헤더의 크기

typedef struct _tgPacketHeader
{
	DWORD	PktID;
	WORD	PktSize;
}PACKETHEADER;

#define PKT_MOUSPOSITION		0x00100011
typedef struct _tgMousePosition : PACKETHEADER
{
	int x;
	int y;
}MOUSEPOSITION;

#define PKT_LOGIN				0xa0000001
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

#define PKT_LOGINRESULT			0x00110022
typedef struct _tgLoginResult : PACKETHEADER
{
	eLoginResut result;
} ST_LOGIN_RESULT;

#define PKT_LOBBYINFOREQ	   0xff000011
typedef struct _tgLobbyInfoReq : PACKETHEADER
{
} ST_LOBBYINFOREQ;

#define PKT_LOBBYINFORES	   0xff000012
typedef struct _tgLobbyInfoRes : PACKETHEADER
{
	char name[30];
} ST_LOBBYINFORES;

#define PKT_LOBBYENDOFROOMINFO	0xff000013
typedef struct _tgLobbyEndOfRoomInfo : PACKETHEADER
{
} ST_LOBBY_END_OF_ROOM_INFO;

#define PKT_CREATEROOMREQ		0x00220010
typedef struct _tgCreateRoom : PACKETHEADER
{
	char title[50];
} ST_ROOM_CREATE_REQ;

#define PKT_CREATEROOMRES		0x00220011
typedef struct _tgCreateRoomRes : PACKETHEADER
{
	DWORD roomNum;
} ST_ROOM_CREATE_RES;

#define PKT_ROOMINFOREQ			0x00220012
typedef struct _tgRoomInfoReq : PACKETHEADER
{
} ST_ROOMINFOREQ;

#define PKT_ROOMINFORES			0x00220013
typedef struct _tgRoomInfoRes : PACKETHEADER
{
	DWORD index;
	unsigned char max;
	unsigned char cur;
	char title[50];
	char masterName[30];
}ST_ROOM_INFORES;

enum eEnterRoomResult
{
	EnterRoomSuccess = 0,
	EnterRoomFailByNoRoom = 1,
	EnterRoomFailByFull = 2,
	EnterRoomFailByExistUser = 3
};

#define PKT_ENTERROOMREQ		0x00220014
typedef struct _tgUserEnterRoomReq : PACKETHEADER
{
	DWORD roomNum;
} ST_ENTER_ROOM_REQ;

#define PKT_ENTERROOMRES		0x00220015
typedef struct _tgUserEnterRoomRes : PACKETHEADER
{
	eEnterRoomResult result;
} ST_ENTER_ROOM_RES;


#define PKT_EXITROOMREQ			0x00220016
typedef struct _tgUserExitRoomReq : PACKETHEADER
{
} ST_EXIT_ROOM_REQ;

#define PKT_EXITROOMRES			0x00220017
typedef struct _tgUserExitRoomRes : PACKETHEADER
{
} ST_EXIT_ROOM_RES;

#define PKT_STARTGAMEREQ		0x00440010
typedef struct _tgStartGameReq : PACKETHEADER
{
} ST_START_GAME_REQ;

#define PKT_STARTGAMERES		0x00440011
typedef struct _tgStartGameRes : PACKETHEADER
{
} ST_START_GAME_RES;

#define PKT_ENDGAMEREQ		0x00440012
typedef struct _tgEndGameReq : PACKETHEADER
{
} ST_END_GAME_REQ;

#define PKT_ENDGAMERES		0x00440013
typedef struct _tgEndGameRes : PACKETHEADER
{
} ST_END_GAME_RES;


///
#define PKT_ENEMYSUPERGUARD	0x00ff0000
#define PKT_RESTART			0x00ff1111

#define PKT_MOVE			0x00ff0011
#define PKT_SHOOT			0x00ff0022
#define PKT_BOOM			0x00ff0023
#define PKT_COLLISION		0x00ff0033
typedef struct _tgPositionInfo : PACKETHEADER
{
	int x;
	int y;
}POSITIONINFO;

#pragma pack(pop)

#endif
