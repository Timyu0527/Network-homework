#define _CRT_SECURE_NO_WARNINGS
#pragma comment (lib, "ws2_32.lib") 
#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>

class Client {
	private:
		bool exitFlag;
	public:
		Client() :exitFlag(0) {}

		void run() {
			WSAData wsaData;

			if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
				fprintf(stderr, "WSAStartup failed.\n");
				exit(1);
			}

			SOCKET sock;
			PCWSTR src = TEXT("127.0.0.1");
			sockaddr_in addr;
			InetPton(AF_INET, src, &addr.sin_addr.s_addr); // 設定 IP
			addr.sin_family = AF_INET;
			addr.sin_port = htons(1234); // 設定 port,htons()跟網路位元組順序有關

			sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

			connect(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR));

			char szBuffer[MAXBYTE] = { 0 };
			recv(sock, szBuffer, MAXBYTE, NULL);
			printf("Message form server: %s\n", szBuffer);
			std::cout << "Please guess a number between 0 and 999." << std::endl;

			std::string num;

			while (!exitFlag) {
				char msg[MAXBYTE] = {};
				std::cout << "Number: ";
				std::cin >> num;
				send(sock, num.c_str(), (int)num.size(), NULL);
				if (num == "exit") {
					exitFlag = 1;
					continue;
				}

				int len = recv(sock, msg, MAXBYTE, NULL);
				if (len <= 0) continue;
				std::cout << "Message form server: " << msg << std::endl;
				if (!strcmp(msg, "bingo!")) {
					exitFlag = 1;
					continue;
				}
			}

			closesocket(sock);

			WSACleanup();

			system("pause");

		}
};

int main() {
	Client* client = new Client();
	client->run();
	return 0;
}