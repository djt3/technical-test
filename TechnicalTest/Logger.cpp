#include "pch.h"
#include "Logger.h"
#include "FilenameHelpers.h"
#include "FileOperations.h"
#include "DataConversion.h"

Logger::Logger()
{
	::InitializeCriticalSection(&m_CriticalSectionFile);
	m_flgIsOpen = false;
	m_flgNoOpenCloseOutput = false;
	m_flgDeleteLogFileOnExit = false;
	m_pRecordOfLogsToDeleteOnClose = nullptr;
}

Logger::~Logger()
{
}

bool Logger::OpenLogFile(bool bClearContents)
{
	if (m_flgIsOpen)
	{
		CloseLogFile();
	}

	m_flgIsOpen = false;
	if (bClearContents)
	{
		CString strBaseSuffix = FilenameHelpers::GetSuffix(m_strLogFile);
		CString strFilter = m_strLogFile;
		CString strMask(_T("*.") + strBaseSuffix);
		FilenameHelpers::ReplaceSuffix(&strFilter, strMask);

		CStringArray aryArchiveLogs;
		CFileFind find;
		BOOL bWorking = find.FindFile(strFilter);
		while (bWorking)
		{
			bWorking = find.FindNextFile();
			if (find.IsDirectory())
				continue;
			aryArchiveLogs.Add(find.GetFilePath());
		}


		this->SortArray(&aryArchiveLogs);

		if (m_pRecordOfLogsToDeleteOnClose)
			m_pRecordOfLogsToDeleteOnClose->Append(aryArchiveLogs);

		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		CString strTry;

		TCHAR* ptchDummy = NULL;
		for (INT32 i = 0; i < aryArchiveLogs.GetSize(); i++)
		{
			strTry = aryArchiveLogs.GetAt(i);

			INT32 nDot = strTry.Left(strTry.GetLength() - strBaseSuffix.GetLength() - 1).ReverseFind('.');

			if (nDot == -1)
				continue;

			CFileStatus stat;
			if (CFile::GetStatus(strTry, stat))
			{
				SYSTEMTIME st;
				stat.m_mtime.GetAsSystemTime(st);
				COleDateTime dtArchive(st);
				COleDateTimeSpan span = dtNow - dtArchive;
				if (span.GetTotalDays() > 62)
				{
					//Reuse this file
					FileOperations::DeleteFileSecure(this, strTry, FILE_DELETE_ALGORITHM_WINDOWS);
				}
			}
		}

		UINT32 nInc = 1;
		bool flgTryFile = true;
		while (flgTryFile)
		{
			CString strInc;
			strInc.Format(_T("%i"), nInc);

			CString strOldFile = m_strLogFile;
			CString strNewSuff(dtNow.Format(_T("%Y_%m_%d_%H_%M_%S_")) + strInc + _T(".") + strBaseSuffix);
			FilenameHelpers::ReplaceSuffix(&strOldFile, strNewSuff);

			if (FileOperations::DoesFileExist(strOldFile))
			{
				nInc++;
				continue;
			}
			else
			{

				if (!FileOperations::MoveFileEx_WithRetry(m_strLogFile, strOldFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING, this, DEFAULT_MOVE_FILE_RETRY_SECS))
					return false;

				if (m_pRecordOfLogsToDeleteOnClose)
					m_pRecordOfLogsToDeleteOnClose->Add(strOldFile);

				break;
			}
		}
	}

	try
	{
		CFileStatus stat;
		if (m_file.GetStatus(m_strLogFile, stat))
		{
			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			CTime ctMod;
			DataConversion::ConvertOleDateTimeToCTime(&dtNow, &ctMod);
			stat.m_mtime = ctMod;

			CFile::SetStatus(m_strLogFile, stat);
		}
	}
	catch (...)
	{

	}


	INT32 nFlags = CFile::shareDenyNone | CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate;


	FileOperations::CreateFullPath(m_strLogFile);

	UINT nTries = 0;
	while (true)
	{
		CFileException exError;
		if (m_file.Open(m_strLogFile, CFile::modeNoInherit | CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyWrite, &exError))
		{
			m_flgIsOpen = true;
			if (!bClearContents)
			{
				m_file.SeekToEnd();
			}

			if (!m_flgNoOpenCloseOutput)
			{
				DWORD dwWrit = 0;
				CString strOpenLog(_T("OpenLog\r\n"));
				m_file.Write(strOpenLog.GetBuffer(0), strOpenLog.GetLength() * sizeof(TCHAR));
			}
			return true;
		}
		else
		{
			nTries++;

			if (nTries == 1)
			{
				this->HandleException(&exError, _T("Opening File"));
			}
			else if (nTries == 5)
			{
				return false;
			}
			else
				Sleep(1000);

		}
	}
}

void Logger::CloseLogFile()
{
	if (m_flgIsOpen)
	{
		m_flgIsOpen = false;
		if (!m_flgNoOpenCloseOutput)
		{
			CString strCloseLog(_T("CloseLog\r\n"));
			m_file.Write(strCloseLog.GetBuffer(0), strCloseLog.GetLength() * sizeof(TCHAR));
		}

		try
		{
			m_file.Close();
		}
		catch (...)
		{
		}
	}
}

CString Logger::GetLogTime()
{
	//Get time
	CString strTime;
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	DWORD dwThreadID = GetCurrentThreadId() % 0x1000;
	strTime.Format(_T("%02i/%02i/%02i%C%02i:%02i:%02i %03X%C"), dtNow.GetDay(), dtNow.GetMonth(), dtNow.GetYear() % 100, ' ', dtNow.GetHour(), dtNow.GetMinute(), dtNow.GetSecond(), dwThreadID, ' ');
	return strTime;
}

void Logger::AddToLogByString(const CString& strMess)
{

	//Construct new line
	CString strLine;
	strLine = this->GetLogTime();

	int nThreadPrefixLen = strLine.GetLength();
	strLine += m_PrefixesForThreads.Get(::GetCurrentThreadId());
	nThreadPrefixLen = strLine.GetLength() - nThreadPrefixLen;

	strLine += strMess;

	strLine += _T("\r\n");
	this->LogToFile(strLine);
}

void Logger::LogToFile(const CString& strMess)
{
	::EnterCriticalSection(&m_CriticalSectionFile);
	if (m_flgIsOpen)
	{
		try
		{
			DWORD dwWrit = 0;
			m_file.Write((LPCTSTR)strMess, strMess.GetLength() * sizeof(TCHAR));
		}
		catch (CFileException* pExFile)
		{
			this->HandleException(pExFile, _T("Adding string"));
		}
	}
	LeaveCriticalSection(&m_CriticalSectionFile);
}

bool Logger::SetLogFile(CString strFileName, bool bClear, CStringArray* pRecordOfLogsToDeleteOnClose, bool flgDeleteLogFileOnExit, DWORD dwLogFileDaysToKeep)
{
	if (strFileName.IsEmpty())
		return false;

	m_strLogFile = strFileName;
	m_flgDeleteLogFileOnExit = flgDeleteLogFileOnExit;
	m_pRecordOfLogsToDeleteOnClose = pRecordOfLogsToDeleteOnClose;

	return this->OpenLogFile(bClear);
}

void Logger::HandleException(CException* pEx, const CString& strContext)
{
	pEx->GetErrorMessage(m_strLastError.GetBuffer(MAX_PATH), MAX_PATH);
	m_strLastError.ReleaseBuffer(-1);
	m_strLastError.Replace(_T("\r"), _T(" "));
	m_strLastError.Replace(_T("\n"), _T(" "));

	if (!m_strLogFile.IsEmpty())
	{
		if (m_strLogFile.Find(_T(".emerg")) == -1)
		{
			Logger Logger;
			Logger.SetLogFile(m_strLogFile + _T(".emerg"));
			Logger.AddToLogByString(_T("ERROR: ") + strContext + _T(". ") + m_strLastError);
		}
	}
}

void Logger::SortArray(CStringArray* paryIn)
{
	CString str1, str2;
	bool flgAnyWorkDone;
	for (INT32 j = 0; j < paryIn->GetSize(); j++)
	{
		flgAnyWorkDone = false;
		for (INT32 i = 0; i < paryIn->GetSize() - 1; i++)
		{
			str1 = paryIn->GetAt(i);
			str2 = paryIn->GetAt(i + 1);

			if (str2 < str1)
			{
				paryIn->SetAt(i, str2);
				paryIn->SetAt(i + 1, str1);

				flgAnyWorkDone = true;
			}
		}

		if (!flgAnyWorkDone)
			break;
	}
}
