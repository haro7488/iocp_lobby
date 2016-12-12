//////////////////////////////////////////////////////////////////////
// CirQueue.cpp
//
//////////////////////////////////////////////////////////////////////

#include <memory.h>
#include "CirQueue.h"


#define PACKET_HEAD_SIZE   6


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCirQueue::CCirQueue()
{
	m_sRear = m_sFront = 0;
	m_pQueue = (char *)malloc(QUEUE_SIZE);

	InitQueue();
}

CCirQueue::~CCirQueue()
{
	free(m_pQueue);	
}

void CCirQueue::InitZeroQueue() 
{ 
	m_sRear = m_sFront = 0;
}

void CCirQueue::InitQueue() 
{ 
	int size;

	size  = m_sRear - m_sFront;
	if(size >= QUEUE_SIZE)
	{
		m_sRear = m_sFront = 0;
		size = 0;
		return;
	}

	memcpy(&m_pQueue[0], &m_pQueue[m_sFront], size);

	m_sFront = 0; 
	m_sRear = size; 
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
int CCirQueue::OnPutData(char *pData, short recvsize)
{
	if( recvsize == 0 ) return 0;
	if( recvsize >= QUEUE_SIZE ) return -1;

	if( (QUEUE_SIZE - m_sRear - 100) <= recvsize ) 
	{
#ifdef _DEBUG
		//char buff[100];
		//wsprintf(buff,"CCirQueue ... InitQueue..... [f : %d, r : %d, size : %d]\n", m_sFront, m_sRear, recvsize);
		//OutputDebugString(buff);
#endif
		InitQueue();
	}
	memcpy(&m_pQueue[m_sRear], pData, recvsize);

	int pre = m_sRear;
	m_sRear += recvsize;

	if( QUEUE_SIZE <= m_sRear)
	{
#ifdef _DEBUG
		//char buff[100];
		//wsprintf(buff,"CCirQueue ... InitQueue..... REAR[f : %d, r : %d, size : %d]\n", m_sFront, m_sRear, recvsize);
		//OutputDebugString(buff);
#endif
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
void CCirQueue::OnPopData(short popsize)
{
	int pre = m_sFront;
	m_sFront += popsize;	
}

PACKETHEADER* CCirQueue::GetPacket()
{
	int packet_size;
	PACKETHEADER *pPacket = NULL;
	int size = m_sRear - m_sFront;
	if(size == 0) return NULL;

	packet_size = ((PACKETHEADER *)&m_pQueue[m_sFront])->PktSize;
	if( size < PACKET_HEAD_SIZE ) return NULL;
	if( size < (packet_size) ) return NULL;

	pPacket = (PACKETHEADER *)&m_pQueue[m_sFront];

	return pPacket;

	return pPacket;
}

