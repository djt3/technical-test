#include "pch.h"
#include "FileOperations.h"
#include "FilenameHelpers.h"
#include "DataConversion.h"

void FileOperations::ListAllFiles(Logger* pLogger, const CString strMask, CStringArray* pstrFiles, bool flgDirsOnly, bool flgDontRecurse, bool flgDirsAlso, DWORD dwListAllFilesFlags)
{

	if (strMask.Left(2) == _T("*."))
	{
		pLogger->AddToLogByString(_T("ERROR: File protection. Attempt to list all files with no specified folder"));
		return;
	}

	CFileFind find;
	CString strMaskToUse;
	if (strMask.Find('*') == -1)
	{
		if (strMask.Right(1) == _T("\\"))
			strMaskToUse = strMask + _T("*.*");
		else
			strMaskToUse = strMask;
	}
	else
		strMaskToUse = strMask;

	BOOL bWorking = find.FindFile(strMaskToUse);

	if (!flgDontRecurse && strMaskToUse.Right(4) != _T("\\*.*"))
	{
		CStringArray aryDirsTemp;
		ListAllFiles(pLogger, FilenameHelpers::GetJustPath(strMaskToUse) + _T("*.*"), &aryDirsTemp, true, true);

		for (int nDir = 0; nDir < aryDirsTemp.GetSize(); nDir++)
		{
			CString strDirWithMask = aryDirsTemp.GetAt(nDir);
			strDirWithMask += FilenameHelpers::GetJustFilename(strMask);
			FileOperations::ListAllFiles(pLogger, strDirWithMask, pstrFiles, flgDirsOnly, flgDontRecurse, flgDirsAlso);
		}
	}

	while (bWorking)
	{
		bWorking = find.FindNextFile();

		if (find.IsDirectory())
		{
			if (!find.IsDots())
			{
				CString strDir = find.GetFilePath();
				if (flgDirsOnly || flgDirsAlso)
					pstrFiles->Add(strDir + _T("\\"));

				if (!flgDontRecurse)
					ListAllFiles(pLogger, strDir + _T("\\"), pstrFiles, flgDirsOnly);
			}
		}
		else
		{
			if (!flgDirsOnly)
			{
				if (dwListAllFilesFlags & LISTALLFILES_IGNORE_HIDDEN)
				{
					CFileStatus statFile;
					CFile::GetStatus(find.GetFilePath(), statFile);
					if (statFile.m_attribute & CFile::hidden)
						continue;
				}
				pstrFiles->Add(find.GetFilePath());
			}
		}
	}
}

bool FileOperations::DeleteFileSecure(Logger* pLogger, const CString& strFile, eFileDeletionAlgorithm nAlgo, DWORD dwTimeoutSecs)
{
	if (strFile.IsEmpty())
	{
		if (pLogger)
			pLogger->AddToLogByString(_T("IMPORTANT: File name is blank."));
		return false;
	}

	if (pLogger)
		pLogger->AddToLogByString(_T("Delete: ") + strFile);

	if (nAlgo == FILE_DELETE_ALGORITHM_WINDOWS)
	{
		return FileOperations::DeleteFile_WithRetry_StandardWindowsAlgo(strFile, pLogger, dwTimeoutSecs) ? true : false;
	}
	else
	{
		if (pLogger)
			pLogger->AddToLogByString(_T("IMPORTANT: Unknown file deletion algorithm."));
		return false;
	}
}

bool FileOperations::DoesFileExist(const CString& strFile, DWORD* pdwWinErr)
{
	DWORD dwRes = ::GetFileAttributes(strFile);
	if (dwRes == -1)
	{
		DWORD dwWinErr = ::GetLastError();
		if (pdwWinErr)
			*pdwWinErr = dwWinErr;

		return false;
	}
	else
		return true;
}

bool FileOperations::CreateFullPath(CString strPath, DWORD* pdwWinErr, INT32 nStartAt, INT32 nRetrySecs)
{
	//Create directory structure on destination
	bool flgLoop = true;
	INT32 nStartPos = nStartAt == -1 ? 6 : nStartAt;
	INT32 nSlash = 0;
	CString strDestFile;
	bool flgFirstPass = true;
	while (flgLoop)
	{
		nSlash = strPath.Find(_T("\\"), nStartPos);

		//carry on to next folder if UNC path
		if (strPath.Left(2) == _T("\\\\") && flgFirstPass)
		{
			nStartPos = nSlash + 1;
			flgFirstPass = false;
			continue;
		}

		if (nSlash == -1)
		{
			flgLoop = false;
		}
		else
		{
			CString strCreateDir = strPath.Left(nSlash);
			if (!strCreateDir.IsEmpty() && strCreateDir.Right(1) != _T(":") && strCreateDir != "\\")
			{
				const auto fnFileOperation = [&strCreateDir]() -> bool {
					return ::CreateDirectory(strCreateDir, NULL);
					};

				const auto fnHandleError = [&pdwWinErr](DWORD dwWinErr, INT32 nTries, bool& bRes) -> bool {
					if (dwWinErr == ERROR_ALREADY_EXISTS || dwWinErr == ERROR_INVALID_NAME) {
						return false;
					}

					if (pdwWinErr) {
						*pdwWinErr = dwWinErr;
					}

					return true;
					};

				PerformFileOperation_WithRetry(fnFileOperation, fnHandleError, nullptr, nRetrySecs, _T(""));
			}
			nStartPos = nSlash + 1;
		}
	}
	return true;
}

bool FileOperations::PerformFileOperation_WithRetry(std::function<bool()> fnOperation, std::function<bool(DWORD dwWinErr, INT32 nTries, bool& bRes)> fnHandleError, Logger* pLogger, INT32 nRetrySecs, const CString& strLogName) {
	bool bRes = false;
	try
	{
		INT32 nTries = 0;
		while (true)
		{
			bRes = fnOperation();
			if (bRes)
			{
				if (pLogger)
				{
					if (nTries > 0)
						pLogger->AddToLogByString(CString(_T("IMPORTANT: ") + strLogName + _T("succeeded after retry.")));
					else
						pLogger->AddToLogByString(CString(strLogName + _T(": success")));
				}
				break;
			}

			DWORD dwWinErr = ::GetLastError();

			if (nTries >= nRetrySecs) {
				if (pLogger)
					pLogger->AddToLogByString(CString(_T("ERROR: ") + strLogName + _T(". WinErr = ") + DataConversion::ConvertDWORDToCString(dwWinErr)));

				break;
			}

			if (!fnHandleError(dwWinErr, nTries, bRes)) {
				break;
			}

			nRetrySecs--;
			Sleep(1000);
		}
	}
	catch (...)
	{
		if (pLogger)
			pLogger->AddToLogByString(_T("ERROR: Hard exception during %s" + strLogName));

		bRes = false;
	}

	return bRes;
}

bool FileOperations::MoveFileEx_WithRetry(const CString& strSource, const CString& strDest, INT32 nFlags, Logger* pLogger, INT32 nRetrySecs)
{
	const auto fnFileOperation = [&strSource, &strDest, nFlags]() -> bool {
		return ::MoveFileEx(strSource, strDest, nFlags);
		};

	const auto fnHandleError = [&strSource, &strDest, &pLogger](DWORD dwWinErr, INT32 nTries, bool& bRes) -> bool {
		if (dwWinErr == ERROR_PATH_NOT_FOUND && nTries == 0)
			FileOperations::CreateFullPath(strDest);
		else if (dwWinErr == ERROR_PATH_NOT_FOUND || dwWinErr == ERROR_FILE_NOT_FOUND)
		{
			if (pLogger)
				pLogger->AddToLogByString(_T("ERROR: Move aborted. WinErr=") + DataConversion::ConvertDWORDToCString(dwWinErr));
			return false;
		}


		if (pLogger)
		{
			CString strMess;
			strMess.Format(_T("IMPORTANT: Move failed. '%s' to '%s'. WinErr=%i"), strSource, strDest, dwWinErr);
			pLogger->AddToLogByString(strMess);
		}

		return true;
		};

	ASSERT(pLogger);

	return PerformFileOperation_WithRetry(fnFileOperation, fnHandleError, pLogger, nRetrySecs, _T("Move"));
}

bool FileOperations::DeleteFile_WithRetry_StandardWindowsAlgo(const CString& strFile, Logger* pLogger, INT32 nRetrySecs)
{
	const auto fnFileOperation = [&strFile]() -> bool {
		return ::DeleteFile(strFile);
		};

	const auto fnHandleError = [&pLogger](DWORD dwWinErr, INT32 nTries, bool& bRes) -> bool {
		if (dwWinErr == ERROR_FILE_NOT_FOUND)
		{
			if (pLogger)
				pLogger->AddToLogByString(_T("Windows API delete: no file"));
			bRes = true;
			return false;
		}
		else if (dwWinErr != ERROR_SHARING_VIOLATION)
		{
			if (pLogger)
				pLogger->AddToLogByString(_T("IMPORTANT: Windows API delete. Will retry. WinErr=") + DataConversion::ConvertDWORDToCString(dwWinErr));
		}

		return true;
		};

	if (pLogger)
		pLogger->AddToLogByString(_T("Using Windows API"));

	return PerformFileOperation_WithRetry(fnFileOperation, fnHandleError, pLogger, nRetrySecs, _T("Windows API delete"));
}

