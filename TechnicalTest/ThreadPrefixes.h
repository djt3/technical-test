#pragma once

class ThreadPrefix
{
public:
	DWORD dwThreadID;
	CString strPrefix;

	ThreadPrefix()
	{
		dwThreadID = 0;
	}
};

class ThreadPrefixes
{
	CArray<ThreadPrefix*, ThreadPrefix*> m_aryThreads;
	CRITICAL_SECTION m_csArray;

public:
	ThreadPrefixes();
	CString Get(DWORD dwThreadID);
	void Set(DWORD dwThreadID, const CString& strPrefix);
};

