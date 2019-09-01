#pragma once
#include <Windows.h>
#define DLL_PUBLIC __declspec(dllexport)

///打开指定路径程序，最大化并返回句柄
DLL_PUBLIC HWND __cdecl wp_exec(char const* exefile, char const* params);

//获取通过0x052C创建的WorkerW hwnd
DLL_PUBLIC HWND __cdecl wp_get_workerw();

//将指定HWND设为WorkerW的子窗口
DLL_PUBLIC int __cdecl wp_setup(HWND wnd);

//恢复
DLL_PUBLIC int __cdecl wp_unsetup(HWND wnd);

//设置窗口位置
DLL_PUBLIC int wp_set_winpos(HWND wnd, long left, long top, long right, long bottom);