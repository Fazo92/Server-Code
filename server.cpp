#include "server.h"

//server::server() {
//}
using namespace std;
SOCKET server::createUDPSocket(int portNumber)
{
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int ws0k = WSAStartup(version, &data);
	if (ws0k != 0) {
		std::cout << "Can't start Winsock! " << ws0k;
	}
	SOCKET in = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (in < 0) perror("Can't create Socket");
	sockaddr_in serverHint;

	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	//serverHint.sin_addr.s_addr = INADDR_ANY;
	serverHint.sin_port = htons(portNumber);

	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == -1) {
		//cout << "Can't bind socket!" << WSAGetLastError() << endl;
		perror("bind failed");
	}
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(in, (sockaddr*)&client, &clientSize);
	char host[NI_MAXHOST];
	char service[NI_MAXHOST];

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);

	if (getnameinfo((sockaddr*)&serverHint, sizeof(serverHint), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		std::cout << host << "connected on port " << service << std::endl;
	}
	else {
		inet_ntop(AF_INET, &serverHint.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connected on port " << ntohs(serverHint.sin_port) << std::endl;
	}
	//else {
	//	cout << "Erfolgreich Verbunden UDP" << endl;
	//}

	return clientSocket;

	//WSACleanup();
}

SOCKET server::createSocket(int port) {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int ws0k = WSAStartup(ver, &wsData);
	if (ws0k != 0) {
		std::cerr << "Can't initialize winsock" << std::endl;
	}
	//Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Can't create a socket" << std::endl;

	}

	sockaddr_in hint;
	//hint.sin_addr.s_addr = inet_addr("127.0.0.1");
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));
	listen(listening, SOMAXCONN);

	//Wait for a connection
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

	//SOCKET clientSocket = accept(listening, NULL, NULL);

	char host[NI_MAXHOST];
	char service[NI_MAXHOST];

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		std::cout << host << "connected on port " << service << std::endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
	}
	closesocket(listening);
	return clientSocket;
}

SOCKET server::createClientSocket(int port) {
	string ipAddress = "10.42.0.1";			// IP Address of the server

	// Initialize WinSock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
	}

	// Create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
	}

	// Fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to server
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		
	}
	return sock;

}