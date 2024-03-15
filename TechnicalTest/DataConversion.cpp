#include "pch.h"
#include "DataConversion.h"

CString DataConversion::ConvertIntToString(const DWORD dwInt)
{
	CString str;
	str.Format(_T("%i"), dwInt);
	return str;
}

CString DataConversion::ConvertDWORDToCString(DWORD64 dwVal)
{
	CString str;
	str.Format(_T("%I64u"), dwVal);
	return str;
}

bool DataConversion::ConvertOleDateTimeToCTime(COleDateTime* pledt, CTime* pctime)
{
	SYSTEMTIME st;
	pledt->GetAsSystemTime(st);
	*pctime = st;
	return true;
}