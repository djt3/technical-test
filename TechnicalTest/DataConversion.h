#pragma once
class DataConversion
{
public:
	static CString ConvertIntToString(const DWORD dwInt);
	static CString ConvertDWORDToCString(DWORD64 dwVal);
	static bool ConvertOleDateTimeToCTime(COleDateTime* pledt, CTime* pctime);
};

