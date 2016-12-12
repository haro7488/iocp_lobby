//////////////////////////////////////////////////////////////////////
// SendPacket.cpp
//////////////////////////////////////////////////////////////////////

#include "SendPacket.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSendPacket::CSendPacket()
{
	m_hSocket = INVALID_SOCKET;
	m_ret = 0;
	m_WSABUF.len = SEND_BUFF_SIZE;
	m_WSABUF.buf = m_SendBuff;

	ClearSendBuff();
}

CSendPacket::~CSendPacket()
{

}

void CSendPacket::ClearSendBuff()
{
	memset(&m_SendBuff, 0, SEND_BUFF_SIZE);	
}

void CSendPacket::SetSendBuffer(int size, char *pBuffer)
{
	m_WSABUF.len = size;
	m_WSABUF.buf = pBuffer;
}

void CSendPacket::ResetSendBuffer()
{
	m_WSABUF.len = SEND_BUFF_SIZE;
	m_WSABUF.buf = m_SendBuff;
}

