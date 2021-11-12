#include <iostream>
#include <Windows.h>
#include "LibWallPaper.h"
#include "HookDesktopWin.h"
#include<stdlib.h>

HWND hd;

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		if (argc > 2)
		{
			hd = wp_exec(argv[1], argv[2]);
		}
		else
		{
			hd = wp_exec(argv[1], "");
		}
	}
	else
	{
		hd = wp_exec("C:/Program Files (x86)/DAUM/PotPlayer/PotPlayerMini.exe", "");
		//hd = wp_exec("E:/LE/Unity/DeskTop/Sakura_DesktopMascot/bin/Sakura_DesktopMascot.exe", "");
	}

	if (IsWindow(hd))
	{
		InstallHook(hd);
		wp_setup(hd);

	}
	std::cout << "Hello World!\n" << GetLastError() << std::endl;
	while (true)
	{
		Sleep(10);
		if (getchar() == 'q') {
			break;
		}
	}
	//PostThreadMessage(0x00000B18, WM_USER, GetCurrentThreadId(), 0);
	//MSG ms;
	//while (int ret = GetMessage(&ms, NULL, 0, 0)) 
	//{ 
	//	if (ret != -1) {
	//		TranslateMessage(&ms);
	//		DispatchMessage(&ms);
	//	}
	//}
	atexit([]() {
			UninstallHook();
			wp_unsetup(hd);
			SendMessage(hd, WM_CLOSE, 0, 0);
		});
	UninstallHook();
	wp_unsetup(hd);
	SendMessage(hd, WM_CLOSE, 0, 0);
}