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
		case CTRL_CLOSE_EVENT://�ͷ�����socket
			for (unsigned int i = 0; i < allSockets.fd_count; i++)
			{
				closesocket(allSockets.fd_array[i]);
			}
			//���������
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
	if (2 != HIBYTE(wdsockMsg.wVersion) || 2 != LOBYTE(wdsockMsg.wVersion))
	{
		//����
		WSACleanup();
		return 0;
	}
	//������socket
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
	//����
	if (SOCKET_ERROR == listen(socketServer, 20))
	{
		int a = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}
	//����
	FD_ZERO(&allSockets);
	//װ�������
	FD_SET(socketServer, &allSockets);
	

	while (1)
	{
		SetConsoleCtrlHandler(fun, TRUE);
		fd_set tempSockets = allSockets;
		fd_set writeSockets = allSockets;//socketserverҲ������
		//FD_CLR(socketServer, &writeSockets);���ô˳�ȥsocketServer
		fd_set errorSockets = allSockets;//���ش���socket
		struct timeval st;
		st.tv_sec = 3;
		st.tv_usec = 0;
		int nRes = select(0, &tempSockets, &writeSockets, &errorSockets, &st);
			if(0 == nRes)
			{
				//����Ӧ
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
						//��Ѱ����ʧ��
						printf("�޷��õ�������Ϣ\n");
					}
					printf(optval);

				}
				for (unsigned int i = 0; i < writeSockets.fd_count; i++)//��ͻ��˷�����Ϣ
				{
					//printf("oko");
					/*char buf[1500] = {'okok'};
					
					if (SOCKET_ERROR == send(writeSockets.fd_array[i], buf, strlen(buf), 0))
					{
						int a = WSAGetLastError();
					}*/
				}
				//����Ӧ,������
				for (unsigned int i = 0; i < tempSockets.fd_count; i++)
				{
					if (tempSockets.fd_array[i] == socketServer)//������accept
					{
						SOCKET clientSocket = accept(socketServer, NULL, NULL);
						if (INVALID_SOCKET == clientSocket)//accept���ӳ���
						{
							continue;
						}
						FD_SET(clientSocket, &allSockets);
					}
					else//�ͻ���
					{
						char buf[1500] = { 0 };
						int nRecv = recv(tempSockets.fd_array[i], buf, sizeof(buf), 0);
						if (0 == nRecv)//�ͻ�������
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
							case 10054://ǿ������
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
				//�������ݴ����������Ӧ����,��һ��break
				break;
			}
	}

	
	//���
	for (unsigned int i = 0; i < allSockets.fd_count; i++)
	{
		closesocket(allSockets.fd_array[i]);
	}
	//ɾ��ָ��socket
	FD_CLR(socketServer, &allSockets);
	//�ж�һ��socket�Ƿ��ڼ����У��ڷ��ط�0�����ڷ���0
	FD_ISSET(socketServer, &allSockets);




	closesocket(socketServer);
	WSACleanup();
	system("pause");
	return 0;
}