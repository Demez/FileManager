#pragma once

//#include <Windows.h>
#include <ShObjIdl_core.h>

#include "string.h"

extern IContextMenu *g_context;

void WIN_PrintLastError(const char* format, ...);
HWND WIN_GetHWND();

// Init Funcs
int WIN_LoadShell32();

std::wstring StrToWStr(const std::string& str);





