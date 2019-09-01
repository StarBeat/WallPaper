#include <iostream>
#include <Windows.h>
#include "LibWallPaper.h"
#include "..//HookDesktopWin/HookDesktopWin.h"
#include<stdlib.h>

HWND hd;
int main()
{
	hd = wp_exec("C:/Program Files (x86)/DAUM/PotPlayer/PotPlayerMini.exe","");
	
	InstallHook(hd);
	//wp_setup(hd);
    std::cout << "Hello World!\n";	
	atexit([]() {
		UninstallHook();
		wp_unsetup(hd);
		});
	while (true)
	{

	}

	UninstallHook();
	wp_unsetup(hd);
}