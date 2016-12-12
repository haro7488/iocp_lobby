//////////////////////////////////////////////////////////////////////
// SendPacket.h
//////////////////////////////////////////////////////////////////////

#ifndef _SENDPACKET_H_
#define _SENDPACKET_H_


#include "packet.h"
#include "global.h"

#define SEND_BUFF_SIZE  10000

class CSendPacket  
{
protected:
	SOCKET				  m_hSocket;
	WSABUF                m_WSABUF; // send buffer..
	char                  m_SendBuff[SEND_BUFF_SIZE];
	int                   m_ret;

public:
	CSendPacket();
	virtual ~CSendPacket();

	void ClearSendBuff();

	SOCKET GetSocket() { return m_hSocket; }

	inline PACKETHEADER *GetSendBuff() { return (PACKETHEADER *)m_SendBuff; }
	WSABUF *GetSendWsaBuf() { return &m_WSABUF; }

	void SetSendBuffer(int size, char *pBuffer);
	void ResetSendBuffer();

	virtual BOOL   OnSendPacket() = 0 {}; //자신의 정보를 나한테 보내는 패킷...
	virtual BOOL   OnSendPacket(WSABUF *pWsaBuff) = 0 {}; // 다른 정보를 나한테 보낸다.
};

#endif
