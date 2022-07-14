#include <iostream>
#include <thread>	
#include <mutex>
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib") //Winsock Library
#define STR_LEN 2048

class Client {
	private:
		std::thread t_send, t_recv;
		bool exitFlag;
		std::mutex cout_mtx;
		std::string username;

		COORD GetCursorPosition() {
			HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO cbsi;
			if (GetConsoleScreenBufferInfo(hStdOut, &cbsi)) {
				return cbsi.dwCursorPosition;
			}
			else {
				// The function failed. Call GetLastError() for details.
				COORD invalid = { -1, -1 };
				return invalid;
			}
		}

		void SetCursorPosition(short x, short y) {
			HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			COORD pos = { x, y };
			SetConsoleCursorPosition(hStdOut, pos);
		}

		void clearRow(int y) {
			SetCursorPosition(0, y);
			std::cout << "                            ";
			SetCursorPosition(0, y);
		}

		void send_msg(SOCKET clientSock) {
			while (!exitFlag) {
				std::lock_guard<std::mutex> lock(cout_mtx);
				char msg[STR_LEN] = {};
				std::cin.getline(msg, STR_LEN);
				COORD pos = GetCursorPosition();
				clearRow(pos.Y - 1);
				std::cout << username << ": " << msg << std::endl;
				send(clientSock, msg, (int)strlen(msg), NULL);
				if (!strcmp(msg, "exit")) {
					exitFlag = 1;
					t_recv.detach();
					closesocket(clientSock);
				}
			}
		}

		void recv_msg(SOCKET clientSock) {
			while (!exitFlag) {
				char msg[STR_LEN] = {};
				int len = recv(clientSock, msg, STR_LEN, NULL);
				if (len <= 0 && !exitFlag) {
					std::cout << "disconnect" << std::endl;
					exitFlag = 1;
					t_recv.detach();
					closesocket(clientSock);
				}
				std::cout << msg << std::endl;
			}
		}
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

			char szBuffer[MAXBYTE] = { 0 }, str[STR_LEN] = {};
			int len = 0;
			std::cout << "Waiting for connection..." << std::endl;
			while (len <= 0) {
				connect(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR));
				len = recv(sock, szBuffer, MAXBYTE, NULL);
			}
			std::cout << "Message from server: " << szBuffer << std::endl;

			char usrname[STR_LEN] = {};
			do {
				memset(usrname, 0, sizeof(usrname));
				std::cout << "Set your username: ";
				std::cin.getline(usrname, STR_LEN);
				send(sock, usrname, (int)strlen(usrname), NULL);
				memset(szBuffer, 0, sizeof(szBuffer));
				recv(sock, szBuffer, MAXBYTE, NULL);
				std::cout << szBuffer << std::endl;
			} while (!strcmp(szBuffer, "failed"));
			username.assign(usrname);

			std::thread t1(&Client::send_msg, this, sock);
			std::thread t2(&Client::recv_msg, this, sock);

			t_send = std::move(t1);
			t_recv = std::move(t2);

			if (t_send.joinable()) t_send.join();
			if (t_recv.joinable()) t_recv.join();

			WSACleanup();
		}
};



int main() {
	Client* client = new Client();
	client->run();
	return 0;
}