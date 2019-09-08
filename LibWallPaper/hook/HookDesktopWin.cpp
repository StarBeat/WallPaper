#include "HookDesktopWin.h"
#include <stdio.h>

#pragma region internal
#pragma region data
HHOOK _gMsgHook;
#pragma data_seg("SharedData")
HWND _gNotifyWin = NULL;
HWND _gDskWin = NULL;
#pragma data_seg()
#pragma comment(linker,"/section:SharedData,RWS")  
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

void MySetForegroundWnd(HWND hWnd)
{
	HWND hCurWnd = NULL;
	DWORD dwMyID;
	DWORD dwCurID;
	hCurWnd = ::GetForegroundWindow();
	dwMyID = ::GetWindowThreadProcessId(hWnd, NULL); //::GetCurrentThreadId();
	dwCurID = ::GetWindowThreadProcessId(hCurWnd, NULL);
	::AttachThreadInput(dwCurID, dwMyID, TRUE);
	::SetForegroundWindow(hWnd);
	//::AttachThreadInput(dwCurID, dwMyID, FALSE);
	if (IsIconic(hWnd)) //最小化时还原它
		::ShowWindow(hWnd, SW_RESTORE);
}

//void MySetForegroundWnd(HWND hWnd)
//{
//	//必须动态加载这个函数
//	HMODULE hUser32 = GetModuleHandleA("user32");
//	if (hUser32)
//	{
//		void (WINAPI * SwitchToThisWindow)(HWND, BOOL) =
//			(void(WINAPI*)(HWND, BOOL))GetProcAddress(hUser32, "SwitchToThisWindow");
//		if (SwitchToThisWindow)
//		{
//			::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
//			SwitchToThisWindow(hWnd, true);
//			::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
//			FreeLibrary(hUser32);
//		}
//	}
//}

//typedef    void    (WINAPI * PROCSWITCHTOTHISWINDOW)(HWND, BOOL);
//HMODULE hUser32 = GetModuleHandleA("user32");
//PROCSWITCHTOTHISWINDOW SwitchToThisWindow = (PROCSWITCHTOTHISWINDOW)GetProcAddress(hUser32, "SwitchToThisWindow");
LRESULT CALLBACK hook_proc(int code, WPARAM w, LPARAM l)
{
	MSG* ms = (MSG*)l;
	if (NULL != ms && ms->hwnd != NULL)
	{
		SetForegroundWindow(_gNotifyWin);
		//SendMessage(_gNotifyWin, ms->message, ms->wParam, ms->lParam);
		switch (ms->message)
		{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		//case WM_RBUTTONDOWN:
		//case WM_RBUTTONUP:
		//case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_LBUTTONUP:
		{
			RECT rc;
			::GetWindowRect(_gNotifyWin, &rc);

			int cx_screen = ::GetSystemMetrics(SM_CXSCREEN);  //屏幕 宽
			int cy_screen = ::GetSystemMetrics(SM_CYSCREEN);  //     高

			//ms->pt.x -= rc.left;
			//ms->pt.y -= rc.right;
			SendMessage(_gNotifyWin, ms->message, ms->wParam, ms->lParam);
			//ms->pt.x += rc.left;
			//ms->pt.y += rc.right;

	/*		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(100);
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	*/
	//INPUT m_InPut = { 0 };
	////鼠标消息，需将type置为INPUT_MOUSE，如果是键盘消息,将type置为INPUT_KEYBOARD。
	//m_InPut.type = INPUT_MOUSE;
	////将type置为鼠标消息后，其INPUT结构中的mi结构是可以使用的，hi、ki结构不可使用
	//m_InPut.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	//m_InPut.mi.dx = ms->pt.x;
	//m_InPut.mi.dy = ms->pt.y;
	//SendInput(1 , &m_InPut, sizeof(INPUT));

	//char s[50];
	//sprintf_s(s, "x:%d+y:%d", ms->pt.x, ms->pt.y);
	//MessageBoxA(NULL, s, "Title", MB_OK);


		}
		break;
		case WM_ACTIVATE:
			ms->message = WM_NULL;
			break;
		case WM_ACTIVATEAPP:
			ms->message = WM_NULL;
			break;
		}

	}
	//PostMessage(_gNotifyWin, WM_COPYDATA, w, l);
	return CallNextHookEx(_gMsgHook, code, w, l);
}
#pragma endregion

DLL_PUBLIC BOOL InstallHook(HWND hRecvMsgWin)
{
	_gDskWin = FindShellWindow();
	if (_gDskWin == NULL)
	{
		return false;
	}
	_gNotifyWin = hRecvMsgWin;
	DWORD dskPid = 0;
	DWORD tid = ::GetWindowThreadProcessId(_gDskWin, &dskPid);
	if (dskPid == NULL)
	{
		return false;
	}

	_gMsgHook = ::SetWindowsHookEx(WH_GETMESSAGE, hook_proc, (HINSTANCE)_gModule, tid);//指定进程会响应hook_proc回调，非全局需要在WH_MIN WH_MAX之间

	if (_gMsgHook != NULL)
	{
		return true;
	}
	else
	{
		printf("\n InstallHook Error:%d\n", GetLastError());
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