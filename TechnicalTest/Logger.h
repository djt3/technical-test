#pragma once
#include "ThreadPrefixes.h"

class Logger
{
public:
	Logger();
	~Logger();
	void AddToLogByString(const CString& pstrMess);
	void LogToFile(const CString& strMess);
	bool SetLogFile(CString strFileName, bool bClear = false, CStringArray* pRecordOfLogsToDeleteOnClose = NULL, bool flgDeleteLogFileOnExit = false, DWORD dwLogFileDaysToKeep = 7);

private:
	CRITICAL_SECTION m_CriticalSectionFile;
	bool m_flgIsOpen;
	CFile m_file;
	CString m_strLogFile;
	CString m_strLastError;
	CStringArray* m_pRecordOfLogsToDeleteOnClose;
	bool m_flgNoOpenCloseOutput;
	bool m_flgDeleteLogFileOnExit;
	ThreadPrefixes m_PrefixesForThreads;

	bool OpenLogFile(bool bClearContents);
	void CloseLogFile();
	CString GetLogTime();
	void HandleException(CException* pEx, const CString& strContext);
	void SortArray(CStringArray* paryIn);
};

