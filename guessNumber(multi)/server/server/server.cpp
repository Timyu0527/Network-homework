#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <sstream>
#include <string.h>
#include <thread>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <ctime>
#include <winsock2.h> 
#include <Ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib") 
//載入 ws2_32.dll 
#define BUF_SIZE 10086
//直接用std好像會出現bug
#define STR_LEN 2048
//因為socket實際上是int變數,故可直接返回int

class Server {
	private:
		struct Client {
			SOCKET socket;
			std::string username;
			std::thread th;
		};

		std::vector<Client> clients;
		std::vector<std::thread> test;
		std::map<int, std::string> usernames;
		std::set<std::string> members;
		std::mutex cout_mtx, client_mtx;

		std::string ans;
		bool bingo;

		bool usrCheck(std::string usr) {
			return members.find(usr) != members.end();
		}

		void broadcastMsg(SOCKET clientSock, const char* msg) {
			for (Client& i : clients) {
				std::string usrMsg(usernames[(int)clientSock] + " " + msg);
				send(i.socket, usrMsg.c_str(), (int)usrMsg.size(), NULL);
			}
		}

		void disconnect(SOCKET clientSock) {
			for (int i = 0; i < clients.size(); ++i) {
				if (clients[i].socket == clientSock) {
					std::lock_guard<std::mutex> lock(client_mtx);
					std::cout << "server: " << usernames[(int)clientSock] << " has been disconnect" << std::endl;
					clients[i].th.detach();
					clients.erase(clients.begin() + i);
					closesocket(clientSock);
				}
			}
		}

		void addUser(SOCKET clientSock, std::string user) {
			std::lock_guard<std::mutex> lock(client_mtx);
			char username[STR_LEN] = {};
			while (usrCheck(user)) {
				send(clientSock, "failed", 7, NULL);
				int strLen = recv(clientSock, username, STR_LEN, 0);
				user.clear();
				user.assign(username);
			}
			for (Client& i : clients) {
				if (i.socket == clientSock) i.username = user;
			}
			std::string rep = user + " added!";
			send(clientSock, rep.c_str(), rep.size(), NULL);
			std::cout << "server: " << user << " added" << std::endl;
			usernames[(int)clientSock] = user;
			members.insert(user);
		}

		void startConnect(SOCKET sConnect, SOCKADDR* clientAddr, int* clientAddrLen) {
			if (sConnect != INVALID_SOCKET) {
				// 有 client 端成功連線過來
				char str[] = "successfully connected";
				char clientIP[STR_LEN];

				inet_ntop(AF_INET, (void*)clientAddr, clientIP, STR_LEN);
				printf("server: got connection from %s\n", clientIP);
				send(sConnect, str, (int)strlen(str), NULL);

				char username[STR_LEN] = {};
				int strLen = recv(sConnect, username, STR_LEN, 0);
				std::string newUsr(username);
				addUser(sConnect, newUsr);

				while (!bingo) {
					char msg[2048] = {};

					int msgLen = recv(sConnect, msg, STR_LEN, 0);
					if (msgLen <= 0) continue;
					//std::cout << "server: got message " << msg << std::endl;

					if (std::string(msg) == ans) {
						//memset(msg, 0, sizeof(msg));
						bingo = 1;
						strcat(msg, " bingo!");
						broadcastMsg(sConnect, msg);
						std::cout << "server: " << usernames[sConnect] << " " << msg << std::endl;
					}
					else {
						//memset(msg, 0, sizeof(msg));
						strcat(msg, " error!");
						std::cout << "server: " << usernames[sConnect] << " " << msg << std::endl;
						broadcastMsg(sConnect, msg);
					}
				}
				disconnect(sConnect);
			}
		}
	public:
		Server() :bingo(0) {}
		void run() {
			WSAData wsaData;

			if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
				fprintf(stderr, "WSAStartup failed.\n");
				exit(1);
			}

			srand((int)time(NULL));
			std::stringstream ss;
			ss << (rand() % 1000);
			ss >> ans;
			std::cout << "answer: " << ans << std::endl;

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
			bind(sListen, (SOCKADDR*)&addr, sizeof(SOCKADDR)); //server端綁定位置

			listen(sListen, 20);
			sockaddr_in clientAddr; // client 端位址資訊
			int clientAddrLen = sizeof(clientAddr);
			while (!bingo) {
				SOCKET sConnect = accept(sListen, (SOCKADDR*)&clientAddr, &clientAddrLen);
				std::lock_guard<std::mutex> guard(client_mtx);
				clients.push_back({ sConnect, "",std::thread(&Server::startConnect, this, sConnect,
																(SOCKADDR*)&clientAddr, &clientAddrLen) });
			}

			for (Client& i : clients) {
				if (i.th.joinable()) i.th.join();
			}

			closesocket(sListen);

			WSACleanup();
		}
};




int main() {
	Server* server = new Server();
	server->run();
	return 0;
}