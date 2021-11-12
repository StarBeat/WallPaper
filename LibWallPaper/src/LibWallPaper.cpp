#include "LibWallPaper.h"
#include <stdio.h>
#include <uxtheme.h>
#include <dwmapi.h>

#pragma comment(lib, "User32") 
#pragma comment(lib, "Shell32") 

#define logi(fmt, ...) fprintf(stdout, "Info %s:%d" fmt "\n", __FUNCDNAME__, __LINE__, ##__VA_ARGS__);
#define loge(fmt, ...) fprintf(stderr, "Error %s:%d" fmt "\n", __FUNCDNAME__, __LINE__, ##__VA_ARGS__);

#pragma region inner
#define log_window(prefix, wnd) wp_print_window(prefix, wnd, stderr)

#pragma region globaldata
static	HWND _gParent_Wnd;//保存原本父窗口
long _gOriginStyle;
long _gOriginExStyle;
#pragma endregion


BOOL is_win8_or_later()
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 2;

	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

	return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION,
		dwlConditionMask);
}

void wp_print_window(char const* prefix, HWND wnd, FILE* f)
{
	char classname[512];
	char name[512];
	RECT rect;
	unsigned wndid;

	*classname = 0;
	*name = 0;
	memset(&rect, 0, sizeof(rect));

	/* TODO: can window handles go above the 32-bit range? I doubt it */
	wndid = (unsigned)((size_t)wnd & 0xFFFFFF);
	GetClassNameA(wnd, classname, sizeof(classname) - 1);
	GetWindowTextA(wnd, name, sizeof(name) - 1);
	GetWindowRect(wnd, &rect);

	if (!prefix) {
		prefix = "";
	}

	fprintf(f, "%s[%08X] %40s | %4d, %4d, %4d, %4d | %s\n", prefix,
		wndid, classname, rect.left, rect.top, rect.right, rect.bottom, name);
}

BOOL CALLBACK find_worker(HWND wnd, LPARAM lp)
{
	HWND* pworker = (HWND*)lp;

	if (!FindWindowExA(wnd, 0, "SHELLDLL_DefView", 0)) {
		return TRUE;
	}

	*pworker = FindWindowExA(0, wnd, "WorkerW", 0);
	if (*pworker) {
		//SendMessageA(*pworker, WM_CLOSE, 0, 0); 关闭这个WorkerW 直接作为Progman子窗口也可以
		log_window("wallpaper is ", *pworker);
		log_window("its parent is ", wnd);
		return FALSE;
	}

	return TRUE;
}

int update_window_styles(HWND wnd, long _and, long ex_and, long _or , long ex_or)
{
	unsigned gle;
	long style = 0, exstyle = 0;

	SetLastError(0);

	style = GetWindowLongA(wnd, GWL_STYLE);
	if (!style) goto errcheck;
	exstyle = GetWindowLongA(wnd, GWL_EXSTYLE);
errcheck:
	gle = GetLastError();
	if ((!style || !exstyle) && gle) {
		loge("GetWindowLongA failed, GLE=%08X", GetLastError());
		return -1;
	}

	_gOriginStyle = style;
	_gOriginExStyle = exstyle;

	style &= _and;
	exstyle &= ex_and;
	style |= _or;
	style |= ex_or;
	//MARGINS margins = { -1 };
	//DwmExtendFrameIntoClientArea(wnd, &margins);//设置无边框
	SetLastError(0);

	if (!SetWindowLongA(wnd, GWL_STYLE, style) ||
		!SetWindowLongA(wnd, GWL_EXSTYLE, exstyle))
	{
		gle = GetLastError();
		if (gle) {
			loge("SetWindowLongA failed, GLE=%08X\n", gle);
			return -2;
		}
	}

	return 0;
}

int resume_window_styles(HWND wnd, long _and, long ex_and, long _or, long ex_or)
{
	unsigned gle;
	long style = 0, exstyle = 0;

	SetLastError(0);

	style = GetWindowLongA(wnd, GWL_STYLE);
	if (!style) goto errcheck;
	exstyle = GetWindowLongA(wnd, GWL_EXSTYLE);
errcheck:
	gle = GetLastError();
	if ((!style || !exstyle) && gle) {
		loge("GetWindowLongA failed, GLE=%08X", GetLastError());
		return -1;
	}

	style &= _and;
	exstyle &= ex_and;
	style |= _or;
	style |= ex_or;

	SetLastError(0);

	if (!SetWindowLongA(wnd, GWL_STYLE, style) ||
		!SetWindowLongA(wnd, GWL_EXSTYLE, exstyle))
	{
		gle = GetLastError();
		if (gle) {
			loge("SetWindowLongA failed, GLE=%08X\n", gle);
			return -2;
		}
	}

	return 0;
}

int wp_map_rect(HWND wnd, RECT* mapped)
{
	if (GetWindowRect(wnd, mapped) && !!GetLastError())
	{
		loge("GetWindowRect failed, GLE=%08X", GetLastError());
		return -1;
	}

	MapWindowPoints(0, wp_get_workerw(), (LPPOINT)mapped, 2);
	return 0;
}

struct HWND_PID
{
	HWND hwd;
	DWORD pid;
};

BOOL is_main_window(HWND handle)
{
	//https://stackoverflow.com/questions/1888863/how-to-get-main-window-handle-from-process-id
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL WINAPI EnumWindowsProc(HWND wnd, LPARAM lParam)
{
	HWND_PID* hp = (HWND_PID*)lParam;
	DWORD curPid = 0;
	GetWindowThreadProcessId(wnd, &curPid);
	if (curPid == hp->pid && is_main_window(wnd))
	{
		//HWND pWnd = GetParent(wnd);
		hp->hwd = wnd;
		return FALSE;
	}
	hp->hwd = NULL;
	return TRUE;
}

HWND GetWindowByPid(HANDLE hProc)
{
	if (!hProc)return NULL;
	HWND_PID hp;
	hp.pid = ::GetProcessId(hProc);
	EnumWindows(EnumWindowsProc, (LPARAM)&hp);
	if (hp.hwd == NULL)
	{
		Sleep(1000);
		EnumWindows(EnumWindowsProc, (LPARAM)& hp);
	}
	return hp.hwd;
}

#pragma endregion


DLL_PUBLIC HWND wp_exec(char const* exefile, char const* params)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	char cmd[200];
	sprintf_s(cmd, "%s%s", exefile, params);
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.wShowWindow = SW_SHOWMAXIMIZED;
	//si.dwFlags = STARTF_USESHOWWINDOW;
	if (CreateProcessA(
		NULL,   //  指向一个NULL结尾的、用来指定可执行模块的宽字节字符串  
		cmd, // 命令行字符串  
		NULL, //    指向一个SECURITY_ATTRIBUTES结构体，这个结构体决定是否返回的句柄可以被子进程继承。  
		NULL, //    如果lpProcessAttributes参数为空（NULL），那么句柄不能被继承。<同上>  
		false,//    指示新进程是否从调用进程处继承了句柄。   
		CREATE_NEW_PROCESS_GROUP,  //  指定附加的、用来控制优先类和进程的创建的标  
			//  CREATE_NEW_CONSOLE  新控制台打开子进程  
			//  CREATE_SUSPENDED    子进程创建后挂起，直到调用ResumeThread函数  
		NULL, //    指向一个新进程的环境块。如果此参数为空，新进程使用调用进程的环境  
		NULL, //    指定子进程的工作路径  
		&si, // 决定新进程的主窗体如何显示的STARTUPINFO结构体  
		&pi  // 接收新进程的识别信息的PROCESS_INFORMATION结构体  
	))
	{
		logi("create process success");
		if (WaitForInputIdle(pi.hProcess, INFINITE) == 0)
		{
			Sleep(2000);//等待程序启动
			HWND hwd = GetWindowByPid(pi.hProcess);
			return hwd;
		}
		//下面两行关闭句柄，解除本进程和新进程的关系，不然有可能不小心调用TerminateProcess函数关掉子进程  
//      CloseHandle(pi.hProcess);  
//      CloseHandle(pi.hThread);  
	}
	else {
		loge("exec erro!");
	}
	/*
	HANDLE hd =ShellExecuteA(0, "open", exefile, params, 0, SW_SHOWMAXIMIZED);
	if (int(hd) > 32)//返回句柄
	{
		return hd;
	}
	/*
	返回值可能的错误有: = 0 {内存不足}
	ERROR_FILE_NOT_FOUND = 2; {文件名错误}
	ERROR_PATH_NOT_FOUND = 3; {路径名错误}
	ERROR_BAD_FORMAT = 11; {EXE 文件无效}
	SE_ERR_SHARE = 26; {发生共享错误}
	SE_ERR_ASSOCINCOMPLETE = 27; {文件名不完全或无效}
	SE_ERR_DDETIMEOUT = 28; {超时}
	SE_ERR_DDEFAIL = 29; {DDE 事务失败}
	SE_ERR_DDEBUSY = 30; {正在处理其他 DDE 事务而不能完成该 DDE 事务}
	SE_ERR_NOASSOC = 31; {没有相关联的应用程序}
	loge("error code(%08X)", GetLastError());
	*/
	return NULL;
}

DLL_PUBLIC int wp_set_winpos(HWND wnd, long left, long top, long right, long bottom)
{
	BOOL succ = SetWindowPos(wnd, 0, left, top, right - left, bottom - top, 0);
	if (!succ) {
		loge("SetWindowPos failed, GLE=%08X", GetLastError());
		return -1;
	}
	return 0;
}

DLL_PUBLIC HWND wp_get_workerw()
{
	HWND progman;
	HWND worker;

	progman = FindWindowA("Progman", 0);
	if (!progman)
	{
		loge("error code(%08X)" ,GetLastError());
		return 0;
	}
	/*
	 * this is basically all the magic. it's an undocumented window message that
	 * forces windows to spawn a window with class "WorkerW" behind deskicons
	 */

	SendMessageA(progman, 0x052C, 0xD, 0);
	SendMessageA(progman, 0x052C, 0xD, 1);

	EnumWindows(find_worker, (LPARAM)&worker);

	if (!worker)
	{
		logi("W: couldn't spawn WorkerW window, trying old method");
		SendMessageA(progman, 0x052C, 0, 0);

		logi("checking for wallpaper");
		EnumWindows(find_worker, (LPARAM)& worker);
	}

	/*
	 * windows 7 with aero is almost the same as windows 10, except that we
	 * have to hide the WorkerW window and render to Progman child windows
	 * instead
	 */

	if (worker && !is_win8_or_later())
	{
		logi("detected windows 7, hiding worker window");
		ShowWindow(worker, SW_HIDE);
		worker = progman;
	}

	if (!worker)
	{
		loge("worker create faild");
		worker = progman;
	}

	return worker;
}

DLL_PUBLIC int wp_setup(HWND wnd)
{
	char wndclass[512];
	HWND wallpaper = wp_get_workerw();
	long _and, ex_and;
	RECT r{0};

	*wndclass = 0;
	GetClassNameA(wnd, wndclass, sizeof(wndclass) - 1);

	if (wallpaper == wnd || !strcmp(wndclass, "Shell_TrayWnd")) {
		loge("can't add this window\n");
		return -1;
	}

	if (IsChild(wallpaper, wnd)) {
		loge("already added\n");
		return -1;
	}

	log_window("adding ", wnd);
	_gParent_Wnd = GetParent(wnd);
	/*
	 * styles blacklist taken from https://github.com/Codeusa/Borderless-Gaming/
	 * blob/2fef4ccc121412f215cd7f185c4351fd634cab8b/BorderlessGaming.Logic/
	 * Windows/Manipulation.cs#L70
	 */

	 /* TODO: somehow save old styles so we can restore them */

	_and = ~(
		WS_CAPTION |
		WS_THICKFRAME |
		WS_SYSMENU |
		WS_MAXIMIZEBOX |
		WS_MINIMIZEBOX
		);
	ex_and = ~(
		WS_EX_DLGMODALFRAME |
		WS_EX_COMPOSITED |
		WS_EX_WINDOWEDGE |
		WS_EX_CLIENTEDGE |
		WS_EX_STATICEDGE |
		WS_EX_TOOLWINDOW |
		WS_EX_LAYERED |
		WS_EX_APPWINDOW
		)
		;

	if (update_window_styles(wnd, _and, ex_and, WS_CHILD, 0)) {
		return -1;
	}

	/* window retains screen coordinates so we need to adjust them */
	wp_map_rect(wnd, &r);

	if (!SetParent(wnd, wallpaper)) {
		loge("SetParent failed, GLE=%08X", GetLastError());
		return -1;
	}

	ShowWindow(wnd, SW_SHOW);
	wp_set_winpos(wnd, r.left, r.top, r.right, r.bottom);
	return 0;
}

DLL_PUBLIC int  wp_unsetup(HWND wnd)
{
	HWND workerw = wp_get_workerw();
	if (workerw == wnd)
	{
		loge("wnd == workerw!");
		return -1;
	}
	if (resume_window_styles(wnd, _gOriginStyle, _gOriginExStyle, WS_CHILD, 0)) {
		loge("resume_window_styles fail!");
		return -1;
	}
	if (!SetParent(wnd, _gParent_Wnd)) {
		loge("SetParent failed, GLE=%08X", GetLastError());
		return -1;
	}
	InvalidateRect(workerw, 0, 1);
	wp_get_workerw();//刷新worker
	return 0;
}