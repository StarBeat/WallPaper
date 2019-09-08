#pragma once
#include <windows.h>

#define DLL_PUBLIC __declspec(dllexport)

HMODULE _gModule;


DLL_PUBLIC BOOL InstallHook(HWND hRecvMsgWin);
DLL_PUBLIC BOOL UninstallHook();