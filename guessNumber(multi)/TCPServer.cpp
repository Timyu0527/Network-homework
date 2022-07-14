#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>

int main() {
	WSAData wsaData;

	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup failed.\n");
		exit(1);
	}

	SOCKET sListen;
	SOCKET sConnect;
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
	sConnect = accept(sListen, (SOCKADDR*)& clientAddr, &clientAddrLen);
	if (sConnect != INVALID_SOCKET)
	{
		// 有 client 端成功連線過來
		char clientIP[20];
		inet_ntop(AF_INET, (void*)& clientAddr, clientIP, 20);
		printf("server: got connection from %s\n", clientIP);

		const char* str = "Hello World!";
		send(sConnect, str,(int) strlen(str), NULL);
	}

	closesocket(sConnect);
	closesocket(sListen);

	WSACleanup();

	return 0;
}