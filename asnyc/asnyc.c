#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")
#define	UM_ASYNCSELECTMSG WM_USER+1

#define MAX_SOCKCOUNT 1024
SOCKET sockall[MAX_SOCKCOUNT];
int sockcount = 0;

unsigned int ServerPrepare(void);
LRESULT CALLBACK WinBackProc(HWND hWnd, UINT msgID, WPARAM wparam, LPARAM lparam);
int  WINAPI WinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPreInstance,_In_ LPSTR LpCmdLine,_In_ int nShowCmd)
{
	//创建窗口结构体
	WNDCLASSEX wc;
	wc.cbClsExtra = 0;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("PicoWindow");
	wc.cbWndExtra = 0;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WinBackProc;
	wc.hbrBackground = NULL;
	wc.hCursor = NULL;
	wc.hIcon = NULL;
	wc.hIconSm = NULL;
	wc.lpszMenuName = NULL;
	//注册结构体
	RegisterClassEx(&wc);
	//创建窗口
	HWND hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, TEXT("PicoWindow"), TEXT("Pico窗口"),WS_OVERLAPPEDWINDOW, 200, 200, 600, 400, NULL, NULL, hInstance, NULL);
	if (NULL == hWnd)
	{
		return 0;
	}
	//显示窗口
	ShowWindow(hWnd, 1);
	//更新窗口
	UpdateWindow(hWnd);
	//消息循环
	//ServerPrepare();
	SOCKET _SocketServer = ServerPrepare();
	if (SOCKET_ERROR == WSAAsyncSelect(_SocketServer, hWnd, UM_ASYNCSELECTMSG, FD_ACCEPT))
	{
		int a = WSAGetLastError();
		closesocket(_SocketServer);
		WSACleanup();
		return 0;
	}
	sockall[sockcount] = _SocketServer;
	sockcount++;
	MSG msg = {0};
	while (GetMessage(&msg, hWnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	for (int i = 0; i < sockall[sockcount]; i++)//关闭socket
	{
		closesocket(sockall[sockcount]);
	}
	WSACleanup();
	return 0;
}
   int y = 0;//textout输出位置
	LRESULT CALLBACK WinBackProc(HWND hWnd, UINT msgID, WPARAM wparam, LPARAM lparam)
	{
		//参数2是客户端socket，参数3是事件信号
		HDC hDc = GetDC(hWnd);
		switch (msgID)
		{
		case UM_ASYNCSELECTMSG:
			//MessageBox(NULL, L"刘建华牛逼！", L"提示", MB_OK);
			//获取socket
		{
			SOCKET socket = (SOCKET)wparam;

			//获取消息
			if (0 != HIWORD(lparam))
			{
				if (WSAECONNABORTED == HIWORD(lparam))
				{
					TextOut(hDc, 0, y, TEXT("close"), strlen("close"));
					y += 15;
					//关闭该socket上的消息
					WSAAsyncSelect(socket, hWnd, 0, 0);
					//关闭socket
					closesocket(socket);
					//从数组中删除该socket
					for (int i = 0; i < sockcount; i++)
					{
						if (socket == sockall[i])
						{
							sockall[i] = sockall[sockcount - 1];
							sockcount--;
							break;
						}
						
					}
				}
				break;
			}
			//具体消息
			switch (lparam)
			{
			case FD_ACCEPT:
			{
				TextOut(hDc, 0, y, TEXT("accept"), strlen("accept"));
				y += 15;
				SOCKET socketclient = accept(socket, NULL, NULL);
				if (INVALID_SOCKET == socketclient)
				{
					//出错
					int a = WSAGetLastError();
					break;
				}
				//将客户端投递给消息队列
				if (SOCKET_ERROR == WSAAsyncSelect(socketclient, hWnd, UM_ASYNCSELECTMSG, FD_CLOSE | FD_READ | FD_WRITE))
				{
					int a = WSAGetLastError();
					closesocket(socketclient);
					break;
				}
				sockall[sockcount] = socketclient;
				sockcount++;
			}
			break;
			case FD_READ:
			{
				TextOut(hDc, 0, y, TEXT("read"), strlen("read"));
				char buf[1024] = { 0 };
				if (SOCKET_ERROR == recv(socket, buf, _In_ 1023, 0))
				{
					//出错
					break;
				}
				TextOut(hDc, 30, y, buf, strlen(buf));
				y += 15;

			}
			break;
			case FD_WRITE:
				TextOut(hDc, 0, y, TEXT("write"), strlen("write"));
				y += 15;
				if (SOCKET_ERROR == send(socket, "连接成功！\n", sizeof("连接成功！\n"), 0))
				{
					break;
				}
				break;
			case FD_CLOSE:
				TextOut(hDc, 0, y, TEXT("close"), strlen("write"));
				y += 15;
				//关闭该socket上的消息
				WSAAsyncSelect(socket, hWnd, 0, 0);
				//关闭socket
				closesocket(socket);
				//从数组中删除该socket
				for (int i = 0; i < sockcount; i++)
				{
					if (socket == sockall[i])
					{
						sockall[i] = sockall[sockcount - 1];
						sockcount--;
						break;
					}
				}

				;
			}
			break;
		}
		case WM_CREATE://初始化，只执行一次
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		}
		ReleaseDC(hWnd, hDc);
		return DefWindowProc(hWnd, msgID, wparam, lparam);
	}

	unsigned int ServerPrepare(void)
	{
		//打开网络库
		WORD wdVersion = MAKEWORD(2, 2);
		WSADATA wdSockMsg;
		int nRes = WSAStartup(wdVersion, &wdSockMsg);
		if (0 != nRes)
		{
			switch (nRes)
			{
			case WSASYSNOTREADY:
				printf("重启电脑或检查网络库");
				break;
			case WSAVERNOTSUPPORTED:
				printf("更新网络库");
				break;
			case WSAEINPROGRESS:
				printf("重启程序");
				break;
			case WSAEPROCLIM:
				printf("关闭其他后台软件");
				break;
			}
		}
		//校验版本
		if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
		{
			//出错
			WSACleanup();
			return 0;
		}
		//创建服务器socket
		SOCKET SocketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (SOCKET_ERROR == SocketServer)
		{
			int a = WSAGetLastError();
			closesocket(SocketServer);
			WSACleanup();
			return 0;
		}
		//绑定地址与端口bind
		struct sockaddr_in si;
		si.sin_family = AF_INET;
		si.sin_port = htons(11111);
		si.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		if (SOCKET_ERROR == bind(SocketServer, (const struct sockaddr*)&si, sizeof(si)))
		{
			int a = WSAGetLastError();
			closesocket(SocketServer);
			WSACleanup();
			return 0;

		}
		//监听listen
		if (SOCKET_ERROR == listen(SocketServer, 20))//参数二是最大监听数量
		{
			int a = WSAGetLastError();
			closesocket(SocketServer);
			WSACleanup();
			return 0;
		}
		return SocketServer;
	}
	