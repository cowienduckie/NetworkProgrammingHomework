#include "stdio.h"
#include "conio.h"
#include "string.h"
#include "ws2tcpip.h"
#include "winsock2.h"
#include "process.h"
#define BUFF_SIZE 1024
#define SERVER_PORT 2440
#define SERVER_ADDR "127.0.0.1"
#define ENDING_DELIMITER "\r\n"

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported \n");

	char buff[BUFF_SIZE];
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &(serverAddr.sin_addr.s_addr));

	SOCKET connSock;
	connSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(connSock, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		printf("\nError: %d", WSAGetLastError());
		closesocket(connSock);
		return 0;
	}

	char sBuff[BUFF_SIZE], rBuff[BUFF_SIZE];
	int ret;

	strcpy_s(sBuff, "1234");
	strcat_s(sBuff, ENDING_DELIMITER);
	strcat_s(sBuff, "a@b");
	strcat_s(sBuff, ENDING_DELIMITER);
	strcat_s(sBuff, "456");
	//strcat_s(sBuff, ENDING_DELIMITER);

	ret = send(connSock, sBuff, strlen(sBuff), 0); //Sleep(100);
	ret = recv(connSock, rBuff, BUFF_SIZE, 0);
	if (ret < 0)
		printf("Stream test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}

	ret = recv(connSock, rBuff, BUFF_SIZE, 0);
	if (ret < 0)
		printf("Stream test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}
	/*ret = recv(connSock, rBuff, BUFF_SIZE, 0);
	if (ret < 0)
		printf("Stream test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}*/

	closesocket(connSock);
	WSACleanup();

	return 0;
}