#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib,"Ws2_32.lib")

fd_set allSockets;
BOOL WINAPI fun(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
		case CTRL_CLOSE_EVENT://释放所有socket
			for (unsigned int i = 0; i < allSockets.fd_count; i++)
			{
				closesocket(allSockets.fd_array[i]);
			}
			//清理网络库
			WSACleanup();

	}
	return TRUE;
}
int main(void)
{
	WORD wdVersion = MAKEWORD(2, 1);
	WSADATA wdsockMsg;
	int nres = WSAStartup(wdVersion, &wdsockMsg);
	if (0 != nres)
	{
		switch (nres)
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
	if (2 != HIBYTE(wdsockMsg.wVersion) || 2 != LOBYTE(wdsockMsg.wVersion))
	{
		//错误
		WSACleanup();
		return 0;
	}
	//服务器socket
	SOCKET socketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == socketServer)
	{
		int a = WSAGetLastError();
		WSACleanup();
		return 0;
	}
	struct sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port = htons(12348);
	si.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(socketServer, (const struct sockaddr*)&si, sizeof(si)))
	{
		int a = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}
	//监听
	if (SOCKET_ERROR == listen(socketServer, 20))
	{
		int a = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}
	//清零
	FD_ZERO(&allSockets);
	//装入服务器
	FD_SET(socketServer, &allSockets);
	

	while (1)
	{
		SetConsoleCtrlHandler(fun, TRUE);
		fd_set tempSockets = allSockets;
		fd_set writeSockets = allSockets;//socketserver也在其中
		//FD_CLR(socketServer, &writeSockets);可用此除去socketServer
		fd_set errorSockets = allSockets;//返回错误socket
		struct timeval st;
		st.tv_sec = 3;
		st.tv_usec = 0;
		int nRes = select(0, &tempSockets, &writeSockets, &errorSockets, &st);
			if(0 == nRes)
			{
				//无响应
				continue;
			}
			else if (0 < nRes)
			{
				for (unsigned int i = 0; i < errorSockets.fd_count; i++)
				{
					char optval[100]= { 0 };
					int len = 99;
					if (SOCKET_ERROR == getsockopt(errorSockets.fd_array[i], SOL_SOCKET, SO_ERROR, optval, &len))
					{
						//查寻错误失败
						printf("无法得到错误信息\n");
					}
					printf(optval);

				}
				for (unsigned int i = 0; i < writeSockets.fd_count; i++)//向客户端发送消息
				{
					//printf("oko");
					/*char buf[1500] = {'okok'};
					
					if (SOCKET_ERROR == send(writeSockets.fd_array[i], buf, strlen(buf), 0))
					{
						int a = WSAGetLastError();
					}*/
				}
				//有响应,链接上
				for (unsigned int i = 0; i < tempSockets.fd_count; i++)
				{
					if (tempSockets.fd_array[i] == socketServer)//服务器accept
					{
						SOCKET clientSocket = accept(socketServer, NULL, NULL);
						if (INVALID_SOCKET == clientSocket)//accept链接出错
						{
							continue;
						}
						FD_SET(clientSocket, &allSockets);
					}
					else//客户端
					{
						char buf[1500] = { 0 };
						int nRecv = recv(tempSockets.fd_array[i], buf, sizeof(buf), 0);
						if (0 == nRecv)//客户端下线
						{
							SOCKET socketTemp = tempSockets.fd_array[i];
							FD_CLR(tempSockets.fd_array[i], &allSockets);
							closesocket(socketTemp);
						}
						else if (0 < nRecv)
						{
							printf("%d  %s", nRecv, buf);
						}
						else//SOCKET_ERROR
						{
							int a = WSAGetLastError();
							switch (a)
							{
							case 10054://强制下线
							{
								SOCKET socketTemp = tempSockets.fd_array[i];
								FD_CLR(tempSockets.fd_array[i], &allSockets);
								closesocket(socketTemp);
							}
							}

						}
					}
				}
			}
			else
			{
				//出错，根据错误码情况相应处理,不一定break
				break;
			}
	}

	
	//清除
	for (unsigned int i = 0; i < allSockets.fd_count; i++)
	{
		closesocket(allSockets.fd_array[i]);
	}
	//删除指定socket
	FD_CLR(socketServer, &allSockets);
	//判断一个socket是否在集合中，在返回非0，不在返回0
	FD_ISSET(socketServer, &allSockets);




	closesocket(socketServer);
	WSACleanup();
	system("pause");
	return 0;
}