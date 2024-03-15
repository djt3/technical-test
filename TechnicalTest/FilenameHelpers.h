#pragma once
class FilenameHelpers
{
public:
	static CString GetSuffix(const CString& strFullFilename, bool flgIncludeDot = false);
	static bool ReplaceSuffix(CString* pstrShortOrLongFilename, CString& strNewSuffix, bool flgIncludingDot = false, bool flgFindDotFromRightToLeft = false);
	static CString GetJustPath(const CString& strFullFilename, bool flgRemoveTrailingBackspace = false);
	static CString GetJustFilename(const CString& strFullFilename);
};

