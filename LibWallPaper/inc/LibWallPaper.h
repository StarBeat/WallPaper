#pragma once
#include <Windows.h>
#define DLL_PUBLIC __declspec(dllexport)

///��ָ��·��������󻯲����ؾ��
DLL_PUBLIC HWND __cdecl wp_exec(char const* exefile, char const* params);

//��ȡͨ��0x052C������WorkerW hwnd
DLL_PUBLIC HWND __cdecl wp_get_workerw();

//��ָ��HWND��ΪWorkerW���Ӵ���
DLL_PUBLIC int __cdecl wp_setup(HWND wnd);

//�ָ�
DLL_PUBLIC int __cdecl wp_unsetup(HWND wnd);

//���ô���λ��
DLL_PUBLIC int wp_set_winpos(HWND wnd, long left, long top, long right, long bottom);