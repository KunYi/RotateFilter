
#pragma once
#include <windows.h>
#include <Strsafe.h>

enum { DBG_INFO = 0, DBG_DEBUG, DBG_WARRING , DBG_ERROR };
void SetDbgOutLevel(const UINT32 level);
void DbgOut(const UINT32 level, LPCWSTR lpFmt, ...);