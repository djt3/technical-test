#pragma once

struct MoveConfigEntry {
	CString strFolderPath;
	CString strDestination;
	CString strFileExtensions;
	bool bIncludeFiles;
	bool bIncludeDirs;
};

struct DeleteConfigEntry {
	CString strFolderPath;
	CString strFileExtensions;
	int nRetentionDays;
	bool bIncludeFiles;
	bool bIncludeDirs;
	bool bRecurse;

	CArray<CString> arrExcludedExtensions;

	DeleteConfigEntry() = default;

	DeleteConfigEntry(const DeleteConfigEntry& x) {
		*this = x;
	}

	DeleteConfigEntry& operator =(const DeleteConfigEntry& x) {
		strFolderPath = x.strFolderPath;
		strFileExtensions = x.strFileExtensions;
		nRetentionDays = x.nRetentionDays;
		bIncludeFiles = x.bIncludeFiles;
		bIncludeDirs = x.bIncludeDirs;
		bRecurse = x.bRecurse;

		if (!x.arrExcludedExtensions.IsEmpty()) {
			arrExcludedExtensions.Copy(x.arrExcludedExtensions);
		}

		return *this;
	}
};

class ConfigManager {
public:
	static void Initialize();

	static CString strLogFilePath;
	static CArray<MoveConfigEntry> arrMoveConfig;
	static CArray<DeleteConfigEntry> arrDeleteConfig;

private:
	static CString ExtractStringParamater(const CString& strConfig, const CString& strParam);
	static int ExtractIntParameter(const CString& strConfig, const CString& strParam);
};