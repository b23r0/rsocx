#include "stdafx.h"
#include "ThreadArray.h"


ThreadArray::ThreadArray(void)
{
}


ThreadArray::~ThreadArray(void)
{
	m_csThreadList.Enter();
	{
		ThreadList::iterator it = m_TList.begin();
		for(; it != m_TList.end() ; it++)
		{
			delete (*it);
		}
	}
	m_csThreadList.Leave();

}

BOOL ThreadArray::AddThreadTask( LPTHREAD_START_ROUTINE pThreadRoutinue,LPVOID lpParameter )
{
	BOOL bRet = FALSE;
	m_csThreadList.Enter();
	{
		Thread* pThread = new Thread;
		bRet = pThread->Start(pThreadRoutinue,lpParameter);

		if (bRet)
		{
			m_TList.push_back(pThread);
		}
	}
	m_csThreadList.Leave();
	return bRet;
}

BOOL ThreadArray::WaitAllTaskEnd()
{
	BOOL bRet = FALSE;
	m_csThreadList.Enter();
	{
		ThreadList::iterator it = m_TList.begin();
		for(; it != m_TList.end() ; it++)
		{
			bRet = (*it)->WaitForEnd(INFINITE);
			if (bRet)
			{
				delete (*it);
				it = m_TList.erase(it);
				if (it == m_TList.end())
				{
					break;
				}
			}
			else break;
		}
	}
	m_csThreadList.Leave();
	return bRet;
}
void ThreadArray::TerminalAllTask()
{
	m_csThreadList.Enter();
	{
		ThreadList::iterator it = m_TList.begin();
		for(; it != m_TList.end() ;)
		{
			(*it)->Terminate();
			delete (*it);
			it = m_TList.erase(it);
		}
	}
	m_csThreadList.Leave();
}