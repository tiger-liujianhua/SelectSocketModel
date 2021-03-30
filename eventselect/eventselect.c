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
	//�������
	WORD wdVersion = MAKEWORD(2, 2);
	WSADATA wdSockMsg;
	int nres = WSAStartup(wdVersion, &wdSockMsg);
	if (0 != nres)
	{
		switch (nres)
		{
			case WSASYSNOTREADY:
				printf("�������Ի��������");
				break;
			case WSAVERNOTSUPPORTED:
				printf("���������");
				break;
			case WSAEINPROGRESS:
				printf("��������");
				break;
			case WSAEPROCLIM:
				printf("�ر�������̨���");
				break;
		}
	}
	//У��汾
	if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
	{
		//����
		WSACleanup();
		return 0;
	}
	//����������SOCKET
	SOCKET socketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SOCKET_ERROR == socketServer)
	{
		//����
		int a = WSAGetLastError();
		WSACleanup();
		return 0;
	}
	//�󶨵�ַ��˿�
	struct sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port = htons(12348);
	si.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(socketServer, (const struct sockaddr*)&si, sizeof(si)))
	{
		//����
		int a = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}
	
	struct fd_es_set esSet = { 0,{0},{0} };

	if (SOCKET_ERROR == listen(socketServer, 20))//����2Ϊ����������
	{
		//����
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
		//����
		int a = WSAGetLastError();
		//�ر��¼�
		WSACloseEvent(eventServer);
		//�ͷ�
		closesocket(socketServer);
		//����
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
			printf("������;%d\n", a);
			break;
		}
		//if (WSA_WAIT_TIMEOUT == nRES)//����4  ��ʱ
		//{
		//	continue;
		//}
		DWORD nIndex = nRES - WSA_WAIT_EVENT_0;
		//�õ��±��
		 WSANETWORKEVENTS networkevents;
		 if (SOCKET_ERROR == WSAEnumNetworkEvents(esSet.sockall[nIndex], esSet.eventall[nIndex], &networkevents))
		 {
			 int a = WSAGetLastError();
			 printf("������:%d\n", a);
			 break;
		 }
		 if (networkevents.lNetworkEvents & FD_ACCEPT)
		 {
			 if (0 == networkevents.iErrorCode[FD_ACCEPT_BIT])
			 {
				 //��������
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
				 //Ͷ�ݸ�ϵͳ
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
			 //�ر��׽���
			 closesocket(esSet.sockall[nIndex]);
			 esSet.sockall[nIndex] = esSet.sockall[esSet.count - 1];
			 //�ر��¼�
			 WSACloseEvent(esSet.eventall[nIndex]);
			 esSet.eventall[nIndex] = esSet.eventall[esSet.count-1];
			 esSet.count--;
		 }
		 //����if�ṹ�ɻ���switch����else if�ṹ

	}


	//�ر��¼�
	WSACloseEvent(eventServer);
	//�ͷ�
	closesocket(socketServer);
	//����
	WSACleanup();
	system("pause");
	return 0;
}