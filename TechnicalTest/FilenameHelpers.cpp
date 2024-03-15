#include "pch.h"
#include "FilenameHelpers.h"

CString FilenameHelpers::GetSuffix(const CString& strFullFilename, bool flgIncludeDot)
{
	INT32 nLastSlash = strFullFilename.ReverseFind('\\');
	INT32 nDot = strFullFilename.Find('.', nLastSlash + 1);

	if (nDot == -1)
		return CString(_T(""));
	else
	{
		if (flgIncludeDot)
			return strFullFilename.Mid(nDot);
		else
			return strFullFilename.Mid(nDot + 1);
	}
};

bool FilenameHelpers::ReplaceSuffix(CString* pstrShortOrLongFilename, CString& strNewSuffix, bool flgIncludingDot, bool flgFindDotFromRightToLeft)
{
	INT32 nDot = -1;
	if (flgFindDotFromRightToLeft)
		nDot = pstrShortOrLongFilename->ReverseFind('.');
	else
	{
		INT32 nLastSlash = pstrShortOrLongFilename->ReverseFind('\\');
		nDot = pstrShortOrLongFilename->Find('.', nLastSlash + 1);
	}

	if (nDot == -1)
		return false;
	if (flgIncludingDot)
		nDot--;
	if (strNewSuffix.IsEmpty())
		*pstrShortOrLongFilename = pstrShortOrLongFilename->Left(nDot);
	else
		*pstrShortOrLongFilename = pstrShortOrLongFilename->Left(nDot + 1) + strNewSuffix;
	return true;
};

CString FilenameHelpers::GetJustPath(const CString& strFullFilename, bool flgRemoveTrailingBackspace)
{
	INT32 nBS = strFullFilename.ReverseFind('\\');
	return strFullFilename.Left(flgRemoveTrailingBackspace ? nBS : nBS + 1);
}

CString FilenameHelpers::GetJustFilename(const CString& strFullFilename)
{
	INT32 nBS = strFullFilename.ReverseFind('\\');

	if (nBS == CString::StringLength(strFullFilename) - 1) {
		nBS = strFullFilename.Left(CString::StringLength(strFullFilename) - 1).ReverseFind('\\');
	}

	return strFullFilename.Mid(nBS + 1);
}
