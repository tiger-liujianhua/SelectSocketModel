#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

struct fd_es_set
{
	unsigned short count;
	SOCKET sockall[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT eventall[WSA_MAXIMUM_WAIT_EVENTS];
};

BOOL WINAPIfun(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
		break;
	}
	return TRUE;
}
int main(void)
{
	//打开网络库
	WORD wdVersion = MAKEWORD(2, 2);
	WSADATA wdSockMsg;
	int nres = WSAStartup(wdVersion, &wdSockMsg);
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
	if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
	{
		//错误
		WSACleanup();
		return 0;
	}
	//创建服务器SOCKET
	SOCKET socketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SOCKET_ERROR == socketServer)
	{
		//出错
		int a = WSAGetLastError();
		WSACleanup();
		return 0;
	}
	//绑定地址与端口
	struct sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port = htons(12348);
	si.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(socketServer, (const struct sockaddr*)&si, sizeof(si)))
	{
		//出错
		int a = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}
	
	struct fd_es_set esSet = { 0,{0},{0} };

	if (SOCKET_ERROR == listen(socketServer, 20))//参数2为最大监听数量
	{
		//出错
		int a = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}
	WSAEVENT eventServer  = WSACreateEvent();
	if (WSA_INVALID_EVENT == eventServer)
	{
		int a = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
	}
	if (SOCKET_ERROR == WSAEventSelect(socketServer, eventServer, FD_ACCEPT))
	{
		//出错
		int a = WSAGetLastError();
		//关闭事件
		WSACloseEvent(eventServer);
		//释放
		closesocket(socketServer);
		//清理
		WSACleanup();
	}
	
	esSet.eventall[esSet.count] = eventServer;
	esSet.sockall[esSet.count] = socketServer;
	esSet.count++;
	while (1)
	{
		DWORD nRES = WSAWaitForMultipleEvents(esSet.count,esSet.eventall,FALSE, WSA_INFINITE,FALSE);
		if(WSA_WAIT_FAILED == nRES)
		{
			int a = WSAGetLastError();
			printf("出错了;%d\n", a);
			break;
		}
		//if (WSA_WAIT_TIMEOUT == nRES)//参数4  超时
		//{
		//	continue;
		//}
		DWORD nIndex = nRES - WSA_WAIT_EVENT_0;
		//得到下标后
		 WSANETWORKEVENTS networkevents;
		 if (SOCKET_ERROR == WSAEnumNetworkEvents(esSet.sockall[nIndex], esSet.eventall[nIndex], &networkevents))
		 {
			 int a = WSAGetLastError();
			 printf("出错了:%d\n", a);
			 break;
		 }
		 if (networkevents.lNetworkEvents & FD_ACCEPT)
		 {
			 if (0 == networkevents.iErrorCode[FD_ACCEPT_BIT])
			 {
				 //正常处理
				 SOCKET socketClient = accept(esSet.sockall[nIndex], NULL, NULL);
				 if (INVALID_SOCKET == socketClient)
				 {
					 continue;
				 }
				 WSAEVENT wsaClientEvent = WSACreateEvent();
				 if (WSA_INVALID_EVENT == wsaClientEvent)
				 {
					 closesocket(socketClient);
					 continue;
				 }
				 //投递给系统
				 if (SOCKET_ERROR == WSAEventSelect(socketClient, wsaClientEvent, FD_READ | FD_CLOSE | FD_WRITE))
				 {
					 closesocket(socketClient);
					 WSACloseEvent(wsaClientEvent);
					 continue;
				 }
				 esSet.eventall[esSet.count] = wsaClientEvent;
				 esSet.sockall[esSet.count] = socketClient;
				 esSet.count++;
				 printf("accept evenrt\n");
			 }
			 else
			 {
				 continue;
			 }
		 }
		 if (networkevents.lNetworkEvents & FD_READ)
		 {
			 if (0 == networkevents.iErrorCode[FD_READ_BIT])
			 {
				 char strRecv[1500] = { 0 };
				 if (SOCKET_ERROR == recv(esSet.sockall[nIndex], strRecv, 1499, 0))
				 {
					 int a = WSAGetLastError();
					 printf("recv error,error code:%d\n", a);
					 continue;
				 }
				 printf("recv data:%s\n", strRecv);
			 }
			 else
			 {
				 printf("READ erro,error code:%d\n", networkevents.iErrorCode[FD_READ_BIT]);
				 continue;
			 }
		 }
		 if (networkevents.lNetworkEvents & FD_WRITE)
		 {
			 if (0 == networkevents.iErrorCode[FD_WRITE_BIT])
			 {
				 if (SOCKET_ERROR == send(esSet.sockall[nIndex], "connect success", strlen("connect succss"), 0))
				 {
					 int a = WSAGetLastError();
					 printf("send error,error code:%d\n", a);
					 continue;
				 }
				 printf("write event\n");
			 }
			 else
			 {
				 printf("WRITE error,eror code:%d\n", networkevents.iErrorCode[FD_WRITE_BIT]);
				 continue;
			 }
		 }
		 if (networkevents.lNetworkEvents & FD_CLOSE)
		 {/*
			 if (0 == networkevents.iErrorCode[FD_CLOSE_BIT])
			 {

			 }*/
			 printf("client close\n");
			 //关闭套接字
			 closesocket(esSet.sockall[nIndex]);
			 esSet.sockall[nIndex] = esSet.sockall[esSet.count - 1];
			 //关闭事件
			 WSACloseEvent(esSet.eventall[nIndex]);
			 esSet.eventall[nIndex] = esSet.eventall[esSet.count-1];
			 esSet.count--;
		 }
		 //整个if结构可换成switch或者else if结构

	}


	//关闭事件
	WSACloseEvent(eventServer);
	//释放
	closesocket(socketServer);
	//清理
	WSACleanup();
	system("pause");
	return 0;
}