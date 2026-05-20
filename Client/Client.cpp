#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <iostream>
#include <process.h>

#pragma comment(lib, "ws2_32")

using namespace std;

char Buffer[1024] = { 0, };

unsigned WINAPI RecvThread(void* Socket)
{

	SOCKET ServerSocket = *(SOCKET*)Socket;
	while (true)
	{
		int RecvBytes = recv(ServerSocket, Buffer, sizeof(Buffer), 0);
		if (RecvBytes <= 0)
		{
			cout << "recv fail" << endl;
			break;
		}
		cout << "Server" << Buffer << "send" << endl;
	}
	return 0;
}

unsigned WINAPI SendThread(void* Socket)
{
	//책임은 사용하는 사람한테 있다.
	SOCKET ServerSocket = *(SOCKET*)Socket;

	while (true)
	{
		cin.getline(Buffer, sizeof(Buffer));
		int SendBytes = send(ServerSocket, Buffer, sizeof(Buffer), 0);
		if (SendBytes <= 0)
		{
			cout << "recv fail" << endl;
			break;
		}
	}
	return 0;
}

//blocking, synchronous, multiplexing(polling)
int main()
{
	cout << "-------- Client --------" << endl;
	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN	ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSocket));
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(35000);

	connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	//Handle -> OS가 전달해 주는 관리 꼬리표 os의 커널 오브젝트
	HANDLE ThreadHandles[2];

	//non blocking, asynchrous
	ThreadHandles[0] = (HANDLE)_beginthreadex(0, 0, RecvThread, &ServerSocket, 0, 0);
	ThreadHandles[1] = (HANDLE)_beginthreadex(0, 0, SendThread, &ServerSocket, 0, 0);

	//blocking
	WaitForMultipleObjects(2, ThreadHandles, FALSE, INFINITE);

	closesocket(ServerSocket);
	WSACleanup();
	return 0;
}