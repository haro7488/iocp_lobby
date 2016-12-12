//////////////////////////////////////////////////////////////////////
// CirQueue.cpp
//
//////////////////////////////////////////////////////////////////////

#include <memory.h>
#include "CirQueue.h"



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

	if( (QUEUE_SIZE - m_sRear) <= recvsize ) 
	{
		InitQueue();
	}
	memcpy(&m_pQueue[m_sRear], pData, recvsize);

	m_sRear += recvsize;

	return 1;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
void CCirQueue::OnPopData(short popsize)
{
	m_sFront += popsize;	
}

PACKETHEADER* CCirQueue::GetPacket()
{
	int packet_size;
	int size = m_sRear - m_sFront;
	if(size == 0) return NULL;
    if(size < 6) return NULL;

	packet_size = ((PACKETHEADER *)&m_pQueue[m_sFront])->PktSize;
	if( size < (packet_size) ) return NULL;

	return (PACKETHEADER *)&m_pQueue[m_sFront];
}