#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

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
		//���������
		WSACleanup();
		return 0;
	}
	//��������socket
	SOCKET socketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int a = WSAGetLastError();
	if (INVALID_SOCKET == socketServer)
	{
		//��ѯ������
		int a = WSAGetLastError();
		//���������
		WSACleanup();
		return 0;
	}

	struct sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port = htons(11111);
	si.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//connect(sockServer, (struct sockaddr*)&si, sizeof(si));
	if (SOCKET_ERROR == connect(socketServer, (struct sockaddr*)&si, sizeof(si)))
	{
 		int a = WSAGetLastError();
		//���������
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}
	char buf[1500] = { 0 };
	int re = recv(socketServer, buf, 10, 0);
		if (0 == re)
		{
			printf("�����ѶϿ�\n");
		}
		else if (SOCKET_ERROR == re)
		{
			//����
			int a = WSAGetLastError();
		}
		else
		{
			printf("%d   %s", re, buf);
		}
	while (1)
	{
		char buf[1500] = { 0 };
		//scanf("%s", buf);
		//if (SOCKET_ERROR == send(socketServer, buf, strlen(buf), 0))
		//{
		//	//����
		//	int a = WSAGetLastError();
		//}
	
		//int re = recv(socketServer, buf, 10, 0);
		//if (0 == re)
		//{
		//	printf("�����ѶϿ�\n");
		//}
		//else if (SOCKET_ERROR == re)
		//{
		//	//����
		//	int a = WSAGetLastError();
		//}
		//else
		//{
		//	printf("%d   %s", re, buf);
		//}
		scanf("%s", buf);
		if ('0' == buf[0])
		{
			break;
		}
		else if(SOCKET_ERROR == send(socketServer, buf, strlen(buf), 0))
		{
			//����
			int a = WSAGetLastError();
		}
		
	}
	
	//���������
	closesocket(socketServer);
	WSACleanup();

	system("pause");
	return 0;
}