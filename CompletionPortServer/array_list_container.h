//==============================================================
// array_list_container.h

#ifndef _ARRAY_LIST_CONTAINER_H_
#define _ARRAY_LIST_CONTAINER_H_

#include <set>

template <class type>
class CArrayListContainer
{
protected:
	int		m_count; //size
	
	std::set<type *> m_pList; //


	CRITICAL_SECTION  m_CriticalSection;
protected:
	void  Lock() { EnterCriticalSection(&m_CriticalSection); } 
	void  UnLock() { LeaveCriticalSection(&m_CriticalSection); }

public:
	CArrayListContainer(int size);
	virtual ~CArrayListContainer();

	void push(type *pElement);
	type* pop();
};

//------------------------------------------------------------------------------
//
template <class type>
CArrayListContainer<type>::CArrayListContainer(int size)
{
	m_count = size;
	type *p;
	for(int i = 0; i < size; ++i)
	{
		p = new type;
		m_pList.insert(p);
	}		
	
	InitializeCriticalSection(&m_CriticalSection);
}

//------------------------------------------------------------------------------
//
template <class type>
CArrayListContainer<type>::~CArrayListContainer()
{
	std::set<type *>::iterator it = m_pList.begin();
	for( ; it != m_pList.end(); ++it)
	{
		delete *it;
	}

	m_pList.clear();
	
	DeleteCriticalSection(&m_CriticalSection);
}


//------------------------------------------------------------------------------
//
template <class type>
void CArrayListContainer<type>::push(type *pElement)
{
	if(!pElement)
		return;

	Lock();
	m_pList.insert(pElement);
	UnLock();
}


//------------------------------------------------------------------------------
// push 보다 pop 가 먼저 일어남
template <class type>
type* CArrayListContainer<type>::pop()
{
	type *pElement = NULL;

	Lock();

	if(m_pList.size() > 0)
	{
		std::set<type *>::iterator it = m_pList.begin();
		if(it != m_pList.end())
		{
			pElement = (type *)(*it);
			m_pList.erase(it);
		}
		if(pElement == NULL)
		{
			pElement = new type;
		}
	}
	else
	{
		pElement = new type;
	}

	UnLock();

	return pElement;
}


#endif