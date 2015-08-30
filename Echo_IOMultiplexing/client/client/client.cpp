#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <signal.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void ErrorHandling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void exitSignal(int a)
{
	if (a == SIGINT) puts("signal == SIGINT");
	Sleep(1500);
}

int main()
{
	bool connected = false;
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN servaddr;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	ZeroMemory(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = 3357;

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) ErrorHandling("socket() error");

	if (connect(sock, (SOCKADDR*)&servaddr, sizeof(sockaddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	char buf[501], recvbuf[501]; buf[4] = '[';

	connected = true;
	puts("connection complete.....");

	signal(SIGINT, exitSignal);

	printf("input the nickName : ");
	gets(&buf[5]);
	strcpy(&buf[5 + strlen(&buf[5])], "] : ");
	puts("-----------------------------------------");

	int nicklen = strlen(&buf[4]);

	while (1)
	{
		printf("input MSG (exit q/Q) : ");
		gets(&buf[4 + nicklen]);

		if (buf[4 + nicklen] == 'q' || buf[4 + nicklen] == 'Q') break;

		int msglen = strlen(&buf[4]);
		int len = *((int*)&buf[0]) = msglen; // MSG(nick + msg) length

		send(sock, buf, 4 + msglen, 0); // len(4) + msglen + null(1)

		int total = 0;
		while (total < len)
		{
			int recvlen = recv(sock, &recvbuf[total], 500, 0);
			if (recvlen == -1) ErrorHandling("recv() error");
			total += recvlen;
		}
		recvbuf[total] = NULL;
		printf("MSG from server : %s\n", recvbuf);
	}
	*((int*)&buf[0]) = 0;
	send(sock, buf, 4, 0); // send 'Q/q' to server
	puts("terminate...");
	//shutdown(sock, SD_SEND);
	// ������ ������ ����� �ϴ� ���� while loop����
	// Ŭ���̾�Ʈ�� �ڽ��� ������� MSG�� ���̸� �˰������Ƿ� (echo���� Ư����)
	// �ڽ��� �����͸� ������ �޾ƾ� �ϴ���(len) ��Ȯ�ϰ� �˰�����.
	// ����, �����κ��� �����͸� ��� ���۹޴� ��Ŀ������� ���� while���� ���Ե�
	// ��, Ŭ���̾�Ʈ�� �����Ϸ� �Ѵٴ°��� �����κ��� ���� �����͵� ������ �ǹ�.
	// => shutdown�� ���� half-close������ ���� �� �ʿ� ���� �ٷ� �����ع����� ��.

	closesocket(sock);
	WSACleanup();
	system("pause");

	return 0;
}
