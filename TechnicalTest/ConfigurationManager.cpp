#include "pch.h"
#include "ConfigurationManager.h"

CArray<MoveConfigEntry> ConfigManager::arrMoveConfig;
CArray<DeleteConfigEntry> ConfigManager::arrDeleteConfig;

void ConfigManager::Initialize() {
	TCHAR currentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentDir);

	TCHAR szBuffer[1024];
	DWORD dwRet = GetPrivateProfileSection(_T("TechnicalTestConfiguration"), szBuffer, 1024, CString(currentDir) + _T("\\Configuration.ini"));

	DWORD pos = 0;

	while (pos < dwRet) {
		CString str(szBuffer + pos);

		CString strAction = ExtractStringParamater(str, _T("Action"));
		CString strFolderPath = ExtractStringParamater(str, _T("FolderPath"));

		bool bIncludeFiles = str.Find(_T("IncludeFiles")) != -1 ? ExtractIntParameter(str, _T("IncludeFiles")) : true;
		bool bIncludeDirs = str.Find(_T("IncludeDirs")) != -1 ? ExtractIntParameter(str, _T("IncludeDirs")) : false;
		
		if (strAction == _T("Move")) {
			CString strDestination = ExtractStringParamater(str, _T("Destination"));
			CString strFileExtensions = ExtractStringParamater(str, _T("FileExtensions"));

			arrMoveConfig.Add({ strFolderPath, strDestination, strFileExtensions, bIncludeFiles, bIncludeDirs });
		}

		else if (strAction == _T("Delete")) {
			int nRetentionDays = ExtractIntParameter(str, _T("RetentionDays"));
			bool bRecurce = str.Find(_T("Recurse")) != -1 ? ExtractIntParameter(str, _T("Recurse")) : false;

			CString strFileExtensions = str.Find(_T("FileExtensions=")) != -1 ? ExtractStringParamater(str, _T("FileExtensions")) : _T("*");

			DeleteConfigEntry cfg;
			cfg.strFolderPath = strFolderPath;
			cfg.strFileExtensions = strFileExtensions;
			cfg.nRetentionDays = nRetentionDays;
			cfg.bIncludeFiles = bIncludeFiles;
			cfg.bIncludeDirs = bIncludeDirs;
			cfg.bRecurse = bRecurce;

			if (str.Find(_T("ExcludedExtensions")) != -1) {
				CString strExcluded = ExtractStringParamater(str, _T("ExcludedExtensions"));

				int nTokenPos = 0;
				CString strTokenized = strExcluded.Tokenize(_T(","), nTokenPos);

				while (!strTokenized.IsEmpty()) {
					cfg.arrExcludedExtensions.Add(strTokenized);

					strTokenized = strExcluded.Tokenize(_T(","), nTokenPos);
				}
			}

			arrDeleteConfig.Add(cfg);
		}

		pos += CString::StringLength(str) + 1;
	}
}

CString ConfigManager::ExtractStringParamater(const CString& strConfig, const CString& strParam) {
	CString strParamText = strConfig.Mid(strConfig.Find(strParam + _T("=\"")));
	strParamText = strParamText.Mid(CString::StringLength(strParam) + 2);

	strParamText = strParamText.Left(strParamText.Find(L'"'));

	return strParamText;
}

int ConfigManager::ExtractIntParameter(const CString& strConfig, const CString& strParam) {
	CString strParamText = strConfig.Mid(strConfig.Find(strParam + _T("=")));
	strParamText = strParamText.Mid(CString::StringLength(strParam) + 1);

	int nEndPos = strParamText.Find(L' ');
	
	if (nEndPos != -1) {
		strParamText = strParamText.Left(nEndPos);
	}

	return _ttoi(strParamText);
}