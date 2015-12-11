#pragma once

#include "ods.h"

#ifdef LINUX

	#include <pthread.h>

	#define PTHREAD_SUFFIX ,0
	#define INFINITE NULL
	#define WAIT_OBJECT_0 0
	
	typedef pthread_t HANDLE;
	#define WaitForSingleObject 		pthread_join

	#define CRITICAL_SECTION 			pthread_mutex_t
	#define InitializeCriticalSection 	pthread_mutex_init
	#define DeleteCriticalSection 		pthread_mutex_destroy
	#define EnterCriticalSection 		pthread_mutex_lock
	#define LeaveCriticalSection 		pthread_mutex_unlock
	#define TryEnterCriticalSection 	pthread_mutex_trylock

#else
	
	#define PTHREAD_SUFFIX

#endif

class CriticalSection
{
public:
	CriticalSection()
	{
		::InitializeCriticalSection(&m_section PTHREAD_SUFFIX);
	}

	~CriticalSection()
	{
		::DeleteCriticalSection(&m_section);
	}

	void Enter()
	{
		::EnterCriticalSection(&m_section);
	}

	void Leave()
	{
		::LeaveCriticalSection(&m_section);
	}

	bool TryEnter()
	{
		return ::TryEnterCriticalSection(&m_section);
	}

private:
	CRITICAL_SECTION	m_section;
};



class Thread
{
public:
	Thread()
		: m_hThread(NULL)
		, m_dwThreadId(0)
	{
	};

	~Thread()
	{
		if (NULL != m_hThread) 
#ifdef LINUX
			pthread_kill(m_hThread,0);
#else
			::CloseHandle(m_hThread);
#endif
	}

	bool Start(LPTHREAD_START_ROUTINE fnRoutine, LPVOID lpParameter)
	{
		if (NULL != m_hThread) return FALSE;

		#ifdef LINUX
		
		int ret = pthread_create(&m_hThread,NULL,fnRoutine,lpParameter);

		if ( ret != 0 )
		{
			debugLog(_T("pthread_create error %d"),ret);
		}

		return ret == 0;

		#else

		m_hThread = ::CreateThread(NULL, 0, fnRoutine, lpParameter, 0, &m_dwThreadId);
		return (NULL != m_hThread);

		#endif
	}

	bool WaitForEnd(DWORD dwTimeoutMS = INFINITE)
	{
		if (NULL == m_hThread) return TRUE;

		DWORD dwRet = 0;

#ifdef LINUX

		void* retval = NULL;

		timespec joinDelay; 
		joinDelay.tv_nsec = dwTimeoutMS;

		if (dwTimeoutMS == INFINITE)
			dwRet = pthread_join(m_hThread,&retval);
		else
			dwRet = pthread_timedjoin_np(m_hThread, &retval, &joinDelay); 

#else

		dwRet = ::WaitForSingleObject(m_hThread, dwTimeoutMS);

#endif

		if (WAIT_OBJECT_0 == dwRet)
		{
			m_hThread = NULL;
			m_dwThreadId = 0;

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	void Terminate()
	{
#ifdef LINUX
		pthread_kill(m_hThread,0);
#else
		CloseHandle(m_hThread);
#endif
		m_hThread = NULL;
	}
	bool IsRunning()
	{
		return ! WaitForEnd(0);
	}

private:
	HANDLE		m_hThread;
	DWORD		m_dwThreadId;
};
