//=============================================================
// array_list.h

#ifndef _ARRAY_LIST_H_
#define _ARRAY_LIST_H_

#pragma warning(disable : 4786)
#include <list>

#include "global.h"

template <class type>
class CArrayList
{
protected:
	int		m_count; //size
	
	std::list<type *> m_pList; 

	HANDLE            m_hEvent;
	CRITICAL_SECTION  m_CriticalSection;

protected:
	void  WaitEvent();
	void  SetEventList();
	void  ResetEventList();

	void  Lock() { EnterCriticalSection(&m_CriticalSection); } 
	void  UnLock() { LeaveCriticalSection(&m_CriticalSection); }

public:
	CArrayList(int size);
	virtual ~CArrayList();

	void push(type *pElement);
	type* pop();
};

//------------------------------------------------------------------------------
//
template <class type>
CArrayList<type>::CArrayList(int size)
{
	m_count = size;
	m_hEvent = NULL;

	//m_hEvent = CreateEvent(NULL, FALSE/*auto reset*/, FALSE/*init state : nonsignal*/, NULL);
	m_hEvent = CreateEvent(NULL, TRUE/*manual reset*/, FALSE/*init state : nonsignal*/, NULL);
	InitializeCriticalSection(&m_CriticalSection);
}

//------------------------------------------------------------------------------
//
template <class type>
CArrayList<type>::~CArrayList()
{
	type *p;
	while(m_pList.size() > 0)
	{
		p = m_pList.front();
		delete p;
		m_pList.pop_front();
	}	

	CloseHandle(m_hEvent); m_hEvent = NULL;	
	DeleteCriticalSection(&m_CriticalSection);
}

//------------------------------------------------------------------------------
//
template <class type>
void  CArrayList<type>::WaitEvent()
{
	if(m_hEvent != NULL)
		WaitForSingleObject(m_hEvent, 100);//INFINITE); 
}

//------------------------------------------------------------------------------
//
template <class type>
void  CArrayList<type>::SetEventList() 
{ 
	if(m_hEvent != NULL)
		SetEvent(m_hEvent);	
}


//------------------------------------------------------------------------------
//
template <class type>
void  CArrayList<type>::ResetEventList()
{
	if(m_hEvent != NULL)
		ResetEvent(m_hEvent);	
}


//------------------------------------------------------------------------------
// pop 보다 push 가 먼저 일어남
template <class type>
void CArrayList<type>::push(type *pElement)
{
	if(!pElement)
		return;

	Lock();
	m_pList.push_back(pElement);
	UnLock();

	SetEventList();
}


//------------------------------------------------------------------------------
//
template <class type>
type* CArrayList<type>::pop()
{
	type *pElement = NULL;


	while(1)
	{
		Lock();
		if(m_pList.size() > 0)
		{
#ifdef _DEBUG
//if(m_pList.size() > 100)
//  OutputDebugString("[ERROR]너무 많이 쌓였다.[CArrayList]\n");
#endif
			break;
		}
		else
		{
			UnLock();
#ifdef _DEBUG
//OutputDebugString("[ERROR]넣는것 없이 뺀다.[CArrayList]\n");
#endif
			ResetEventList();
			WaitEvent();
		}		
	}

	pElement = m_pList.front();
	if(pElement != NULL)
		m_pList.pop_front();

	UnLock();

	return pElement;
}

#endif