#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h> 
#include<iostream>
#include<string>
#include<thread>
#include <Ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib") 
//載入 ws2_32.dll 
#define BUF_SIZE 10086
//直接用std好像會出現bug
using std::cout;using std::thread;
//因為socket實際上是int變數,故可直接返回int

void testConnet(int sConnect)
{	
		if (sConnect != INVALID_SOCKET)
		{
			while (1)
			{
				// 有 client 端成功連線過來
				cout << sConnect;
				char clientIP[2048];
				int strLen = recv(sConnect, clientIP, 2048, 0);
				printf("server: got connection from %s\n", clientIP);

				const char* str = "Hello World!";
				send(sConnect, str, (int)strlen(str), NULL);
			}
		}
		closesocket(sConnect);
	
}
int main() {
	//int sListen = listenSocket();
	WSAData wsaData;

	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup failed.\n");
		exit(1);
	}

	SOCKET sListen;
	PCWSTR src = TEXT("127.0.0.1");
	sockaddr_in addr;
	InetPton(AF_INET, src, &addr.sin_addr.s_addr); // 設定 IP
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234); // 設定 port,htons()跟網路位元組順序有關

	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//AF_INET:IPv4	AF_INET6:IPv6
	//SOCK_STREAM:TCP  SOCK_DGRAM:UDP
	//IPPROTO_TCP:TCP  IPIROTO_UDP:UDP
	bind(sListen, (SOCKADDR*)& addr, sizeof(SOCKADDR)); //server端綁定位置

	listen(sListen, 20);
	sockaddr_in clientAddr; // client 端位址資訊
	int clientAddrLen = sizeof(clientAddr);
	while (true)
	{
		int sConnect = accept(sListen, (SOCKADDR*)& clientAddr, &clientAddrLen);
		std::thread t(testConnet,sConnect);
		t.detach();
	}
	closesocket(sListen);


	WSACleanup();

	return 0;
}