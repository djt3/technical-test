#include "pch.h"
#include "ThreadPrefixes.h"

ThreadPrefixes::ThreadPrefixes()
{
	::InitializeCriticalSection(&m_csArray);
}

CString ThreadPrefixes::Get(DWORD dwThreadID)
{
	::EnterCriticalSection(&m_csArray);
	ThreadPrefix* pThread;
	for (int nThread = 0; nThread < m_aryThreads.GetSize(); nThread++)
	{
		pThread = m_aryThreads.GetAt(nThread);
		if (pThread->dwThreadID == dwThreadID)
		{
			::LeaveCriticalSection(&m_csArray);
			return pThread->strPrefix;
		}
	}

	::LeaveCriticalSection(&m_csArray);
	return _T("");
}

void ThreadPrefixes::Set(DWORD dwThreadID, const CString& strPrefix)
{
	::EnterCriticalSection(&m_csArray);
	ThreadPrefix* pThread;
	for (int nThread = 0; nThread < m_aryThreads.GetSize(); nThread++)
	{
		pThread = m_aryThreads.GetAt(nThread);
		if (pThread->dwThreadID == dwThreadID)
		{
			pThread->strPrefix = strPrefix;
			::LeaveCriticalSection(&m_csArray);
			return;
		}
	}

	pThread = new ThreadPrefix;
	pThread->dwThreadID = dwThreadID;
	pThread->strPrefix = strPrefix;
	m_aryThreads.Add(pThread);
	::LeaveCriticalSection(&m_csArray);
}
