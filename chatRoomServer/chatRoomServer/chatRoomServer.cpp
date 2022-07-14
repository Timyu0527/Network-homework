#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <mutex>
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
		std::map<int, std::string> usernames;
		std::set<std::string> members;
		std::mutex cout_mtx, client_mtx;
		
		bool usrCheck(std::string usr) {
			return members.find(usr) != members.end();
		}

		void broadcastMsg(SOCKET sConnect, const char* msg, bool mode) {
			for (Client& i : clients) {
				if (mode && i.socket == sConnect) continue;
				if (i.username.empty()) continue;
				send(i.socket, msg, (int)strlen(msg), NULL);
			}
		}

		void disconnect(SOCKET sConnect) {
			for (int i = 0; i < clients.size(); ++i) {
				if (clients[i].socket == sConnect) {
					std::lock_guard<std::mutex> lock(client_mtx);
					std::cout << "server: " << usernames[sConnect] << " has been disconnect" << std::endl;
					clients[i].th.detach();
					clients.erase(clients.begin() + i);
					closesocket(sConnect);
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
			broadcastMsg(clientSock, rep.c_str(), 0);
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

				while (1) {
					char msg[2048] = {};

					int msgLen = recv(sConnect, msg, STR_LEN, 0);
					if (msgLen <= 0) break;
					std::cout << "server: got message from " << usernames[sConnect] << ": " << msg << std::endl;
					std::string usrMsg(usernames[sConnect] + ": " + msg);
					broadcastMsg(sConnect, usrMsg.c_str(), 1);

					if (!strcmp(msg, "exit")) break;
				}
				disconnect(sConnect);
			}
		}
	public:
		Server() {};
		void run() {
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
			bind(sListen, (SOCKADDR*)&addr, sizeof(SOCKADDR)); //server端綁定位置

			listen(sListen, 20);
			sockaddr_in clientAddr; // client 端位址資訊
			int clientAddrLen = sizeof(clientAddr);
			char clientIP[20] = {};

			while (1) {
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