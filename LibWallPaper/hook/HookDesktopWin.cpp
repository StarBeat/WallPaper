#include "HookDesktopWin.h"
#include <stdio.h>

#pragma region internal
#pragma region data
HHOOK _gMsgHook;
#pragma endregion


HWND FindShellWindow()
{
	// Sometimes, we can't find the desktop window when we use this function, but we must   
	// find it's handle, so we do a loop to find it, but at most we find for 10 times.  
	UINT uFindCount = 0;
	HWND hSysListView32Wnd = NULL;
	while (NULL == hSysListView32Wnd && uFindCount < 10)
	{
		HWND hParentWnd = ::GetShellWindow();
		HWND hSHELLDLL_DefViewWnd = FindWindowEx(hParentWnd, NULL, L"SHELLDLL_DefView", NULL);
		hSysListView32Wnd = FindWindowEx(hSHELLDLL_DefViewWnd, NULL, L"SysListView32", L"FolderView");

		if (NULL == hSysListView32Wnd)
		{
			hParentWnd = FindWindowEx(NULL, NULL, L"WorkerW", L"");
			while ((!hSHELLDLL_DefViewWnd) && hParentWnd)
			{
				hSHELLDLL_DefViewWnd = FindWindowEx(hParentWnd, NULL, L"SHELLDLL_DefView", NULL);
				hParentWnd = FindWindowEx(NULL, hParentWnd, L"WorkerW", L"");
			}
			hSysListView32Wnd = FindWindowEx(hSHELLDLL_DefViewWnd, 0, L"SysListView32", L"FolderView");
		}

		if (NULL == hSysListView32Wnd)
		{
			Sleep(1000);
			uFindCount++;
		}
		else
		{
			break;
		}
	}

	return hSysListView32Wnd;
}

LRESULT CALLBACK hook_proc(int code, WPARAM w, LPARAM l)
{
	MSG* msg = (MSG*)l;
	printf("\n hook_proc\n");
	if (NULL != msg && msg->hwnd != NULL)
	{
		switch (msg->message)
		{
		case WM_MOUSEMOVE:
			POINT pt = ((MOUSEHOOKSTRUCT*)l)->pt;
			PostMessage(_gNotifyWin, WM_MOUSEMOVE, MK_CONTROL, MAKELPARAM(pt.x, pt.y));
			break;
		case WM_LBUTTONDBLCLK:
			printf("\nWM_LBUTTONDBLCLK\n");
			PostMessage(_gNotifyWin, WM_LBUTTONDBLCLK, w, l);
			break;
		default:
			break;
		}
	}
	//PostMessage(_gNotifyWin, WM_COPYDATA, w, l);
	return CallNextHookEx(_gMsgHook, code, w, l);
}
#pragma endregion

DLL_PUBLIC BOOL InstallHook(HWND hRecvMsgWin)
{
	printf("\n InstallHook\n");
	HWND dsk= FindShellWindow();
	if (dsk == NULL)
	{
		return false;
	}
	_gNotifyWin = hRecvMsgWin;
	DWORD dskPid = 0;
	DWORD tid = ::GetWindowThreadProcessId(dsk, &dskPid);
	if (dskPid == NULL)
	{
		return false;
	}
	_gMsgHook = ::SetWindowsHookEx(WH_KEYBOARD_LL, hook_proc, (HINSTANCE)_gModule, NULL);

	if (_gMsgHook != NULL)
	{
		return true;
	}

	return false;
}

DLL_PUBLIC BOOL UninstallHook()
{
	if (_gMsgHook != NULL)
	{
		return UnhookWindowsHookEx(_gMsgHook);
	}
	return false;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		printf("DLL attach\n\n");
		_gModule = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}