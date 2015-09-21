#include "precompiled.h"

#include "scriptinterface/ScriptInterface.h"
#include "scriptinterface/ScriptVal.h"

#include "ps/scripting/JSInterface_Firebase.h"

#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
typedef int WSADATA;
typedef int DWORD;
typedef int SOCKET;
// Sketch I don't thing these are right
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define SOCKADDR_IN sockaddr_in
#define SOCKADDR sockaddr
#define MAKEWORD(...) 0
#define WSAStartup(a, b) ((void)a, (void)b, 0)
#define WSACleanup(...)
#define ZeroMemory(addr, n) memset(addr, 0, n)
#define closesocket(sock) close(sock)
#endif
#include <iostream>
#include <string>
using std::cout;
using std::endl;
using std::string;

// BEGIN SCRIPT INTERFACE

template<class T>
inline string to_string(T num) {
	if (num == 0) return "0";

	string str = "";
	bool negative = false;
	if (num < 0) {
		negative = true;
		num = -num;
	}

	T ten = 10;
	while (num > 0) {
		int lastDigit = (int)(num % ten);
		num /= 10;
		str = (char)(lastDigit + 48) + str;
	}

	if (negative) {
		str = "-" + str;
	}

	return str;
}

SOCKADDR http_socket_lookup_addr(string host, int port) {
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;          // We are targeting IPv4
	hints.ai_protocol = IPPROTO_TCP;    // We are targeting TCP
	hints.ai_socktype = SOCK_STREAM;    // We are targeting TCP so its SOCK_STREAM

	struct addrinfo* targetAdressInfo = NULL;
	DWORD getAddrRes = getaddrinfo(host.c_str(), NULL, &hints, &targetAdressInfo);
	if (getAddrRes != 0 || targetAdressInfo == NULL) {
		cout << "Could not resolve the Host Name" << endl;
		throw "";
	}

	SOCKADDR_IN sockAddr;
	sockAddr.sin_addr = ((SOCKADDR_IN*) targetAdressInfo->ai_addr)->sin_addr;
	sockAddr.sin_family = AF_INET;    // IPv4
	sockAddr.sin_port = htons(port);

	freeaddrinfo(targetAdressInfo);
	return *((SOCKADDR*)&sockAddr);
}

SOCKET http_socket_open(SOCKADDR* addr) {
	SOCKET webSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (webSocket == INVALID_SOCKET) {
		cout << "Creation of the Socket Failed" << endl;
#ifdef _WIN32
		int err = WSAGetLastError();
		WSACleanup();
		debug_printf("send failed with error: %d\n", err);
#endif
		return -1;
	}

	//cout << "Connecting..." << endl;
	if (connect(webSocket, addr, sizeof(*addr)) != 0) {
		cout << "Could not connect" << endl;
#ifdef _WIN32
		int err = WSAGetLastError();
		WSACleanup();
		debug_printf("send failed with error: %d\n", err);
#endif
		closesocket(webSocket);
		return -1;
	}
	//cout << "Connected." << endl;
	return webSocket;
}

void http_socket_request(int webSocket, string method, string path, string data) {
	string httpRequest = \
		method + " " + path + " HTTP/1.0\r\n"\
		"Content-Length: " + to_string(data.size()) + "\r\n"
		"\r\n" +
		data;
	//cout << httpRequest << endl;
	int sentBytes = send(webSocket, httpRequest.c_str(), httpRequest.size(), 0);
	if (sentBytes < (int)httpRequest.size() || sentBytes == SOCKET_ERROR) {
		cout << "Could not send the request to the Server" << endl;
#ifdef _WIN32
		int err = WSAGetLastError();
		WSACleanup();
		debug_printf("send failed with error: %d\n", err);
#endif
		return;
	}

	// Receiving and Displaying an answer from the Web Server
// 	char buffer[10000];
// 	ZeroMemory(buffer, sizeof(buffer));
// 	int dataLen;
// 	while ((dataLen = recv(webSocket, buffer, sizeof(buffer), 0) > 0)) {
// 		int i = 0;
// 		while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {
// //			cout << buffer[i];
// 			i += 1;
// 		}
// 	}
}

#ifdef _WIN32
//http://stackoverflow.com/questions/10905892/equivalent-of-gettimeday-for-windows
int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}
#endif

long long usec() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_usec + tv.tv_sec*1000000;
}

void http_socket_init() {
	// Initialize Dependencies to the Windows Socket.
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
		cout << "WSAStartup failed." << endl;
	}

// 	for (int i = 0; i < 10000; i++) {
// 		long long start = usec();
// 		SOCKET webSocket = http_socket_open(&addr);
// 		if (webSocket < 0) {
// 			system("pause");
// 			WSACleanup();
// 			return -1;
// 		}
//
// 		http_socket_request(webSocket, "PUT", "/quentin/"+to_string(i)+".json",
// 			"{\"pro\": true, \"time\": {\".sv\": \"timestamp\"}}");
// 		closesocket(webSocket);
// 		long long end = usec();
// 		std::cout << "Took " << (double)(end - start)/1000000 << " usec" << endl;
// 		//sleep(1);
// 	}

	// Cleaning up Windows Socket Dependencies
//	WSACleanup();
}


SOCKADDR GLOBAL_addr = {};
long long sessionID = 0;

void JSI_Firebase::FirebaseHTTP(ScriptInterface::CxPrivate* UNUSED(pCxPrivate), std::string method, std::string path, std::string data)
{
	path = std::string("/sessions/session-") + to_string(sessionID) + path;

	long long start = usec();
	SOCKET webSocket = http_socket_open(&GLOBAL_addr);
	http_socket_request(webSocket, method, path, data);
	closesocket(webSocket);
	long long end = usec();
	std::cout << "Took " << (double)(end - start)/1000000 << "s" << endl;
}

void JSI_Firebase::RegisterScriptFunctions(ScriptInterface& scriptInterface)
{
	std::cout << "REGISTERING SCRIPT FUNCTION" << std::endl;
	GLOBAL_addr = http_socket_lookup_addr("localhost", 2500);
	scriptInterface.RegisterFunction<void, std::string, std::string, std::string, &JSI_Firebase::FirebaseHTTP>("FirebaseHTTP");
	std::cout << "REGISTERINGED SCRIPT FUNCTION" << std::endl;


	sessionID = usec();
	SOCKET webSocket = http_socket_open(&GLOBAL_addr);
	http_socket_request(webSocket, "POST", "/sessionsList.json",
		std::string("{ \"id\": \"") + std::string("session-") + to_string(sessionID) + std::string("\", \"time\": {\".sv\": \"timestamp\" } } "));
	closesocket(webSocket);
}