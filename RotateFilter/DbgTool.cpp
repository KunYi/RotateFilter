
#include "DbgTool.h"

static UINT32 DBGOUT_LEVEL = DBG_INFO;

void SetDbgOutLevel(const UINT32 level)
{
	DBGOUT_LEVEL = level;
}

void DbgOut(const UINT32 level, LPCWSTR lpFmt, ...)
{
	wchar_t buff[1024 + 3];
	wchar_t* p = buff;
	va_list args;

	if (level < DBGOUT_LEVEL)
		return;

	HRESULT hResult;
	va_start(args, lpFmt);
	hResult = ::StringCbVPrintfW(p, sizeof(buff) - 3, lpFmt, args);
	va_end(args);
	if (hResult == S_OK)
		OutputDebugString(buff);
	else if (hResult == STRSAFE_E_INVALID_PARAMETER)
		OutputDebugString(L"Error DbgOut: Invalid parameter\n");
	else if (hResult == STRSAFE_E_INSUFFICIENT_BUFFER)
		OutputDebugString(L"Error DbgOut: Insufficient buffer\n");
}