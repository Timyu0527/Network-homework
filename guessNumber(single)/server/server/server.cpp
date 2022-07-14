#define _CRT_SECURE_NO_WARNINGS
#pragma comment (lib, "ws2_32.lib") 
#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <ctime>
#include <string>

class Server {
	private:
		std::string ans;
		bool exitFlag;
	public:
		Server() :exitFlag(0) {}

		void run() {
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
			bind(sListen, (SOCKADDR*)&addr, sizeof(SOCKADDR)); //server端綁定位置
			listen(sListen, 20);

			sockaddr_in clientAddr; // client 端位址資訊
			int clientAddrLen = sizeof(clientAddr);
			std::cout << "Waiting for connection..." << std::endl;
			sConnect = accept(sListen, (SOCKADDR*)&clientAddr, &clientAddrLen);
			if (sConnect != INVALID_SOCKET) {
				// 有 client 端成功連線過來
				char clientIP[20];
				inet_ntop(AF_INET, (void*)&clientAddr, clientIP, 20);
				printf("server: got connection from %s\n", clientIP);

				const char* str = "Successfully, connected!";
				send(sConnect, str, (int)strlen(str), NULL);
			}

			srand((int)time(NULL));
			int tmp = rand() % 1000;
			std::stringstream ss;
			ss << tmp;
			ss >> ans;

			std::cout << "answer: " << ans << std::endl;

			while (!exitFlag) {
				char msg[MAXBYTE] = {};
				int len = recv(sConnect, msg, MAXBYTE, NULL);
				if (len <= 0) {
					std::cout << "Something wrong!" << std::endl;
					exitFlag = 1;
					continue;
				}
				std::cout << msg;
				if (!strcmp(msg, ans.c_str())) {
					std::cout << " bingo!" << std::endl;
					strcpy(msg, "bingo!\0");
					exitFlag = 1;
				}
				else {
					std::cout << " error!" << std::endl;
					strcpy(msg, "error!\0");
				}
				send(sConnect, msg, (int)strlen(msg), NULL);
			}

			closesocket(sConnect);
			closesocket(sListen);

			WSACleanup();

			system("pause");
		}
};



int main() {
	Server* server = new Server();
	server->run();
	return 0;
}