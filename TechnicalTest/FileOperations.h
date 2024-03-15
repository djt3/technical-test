#pragma once
#include "Logger.h"

#include <functional>

#define DEFAULT_MOVE_FILE_RETRY_SECS 30

#define LISTALLFILES_IGNORE_HIDDEN 1

enum eFileDeletionAlgorithm
{
	FILE_DELETE_ALGORITHM_WINDOWS = 0
};

class FileOperations
{
public:
	static bool DeleteFileSecure(Logger* pLogger, const CString& strFile, eFileDeletionAlgorithm nAlgo = FILE_DELETE_ALGORITHM_WINDOWS, DWORD dwTimeoutSecs = DEFAULT_MOVE_FILE_RETRY_SECS);
	static bool DoesFileExist(const CString& strFile, DWORD* pdwWinErr = nullptr);
	static bool CreateFullPath(CString strPath, DWORD* pdwWinErr = NULL, INT32 nStartAt = -1, INT32 nRetrySecs = 0);
	static bool PerformFileOperation_WithRetry(std::function<bool()> fnOperation, std::function<bool(DWORD dwWinErr, INT32 nTries, bool& bRes)> fnHandleError, Logger* pLogger, INT32 nRetrySecs, const CString& strLogName);
	static bool MoveFileEx_WithRetry(const CString& strSource, const CString& strDest, INT32 nFlags, Logger* pLogger, INT32 nRetrySecs = 0);
	static void ListAllFiles(Logger* pLogger, CString str, CStringArray* pstrFiles, bool flgDirsOnly = false, bool flgDontRecurse = false, bool flgDirsAlso = false, DWORD dwListAllFilesFlags = 0);

private:
	static bool DeleteFile_WithRetry_StandardWindowsAlgo(const CString& strFile, Logger* pLogger, INT32 nRetrySecs);
};

