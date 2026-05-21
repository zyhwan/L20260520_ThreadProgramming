#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include "ChatPacket.h"
#include "MovePacket.h"
#include "NetUtil.h"

#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <process.h>
#include <conio.h>

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "NetCommon")


using namespace std;

char SendBuffer[1024] = { 0, };
char RecvBuffer[1024] = { 0, };

bool IsRecvThreadRunning = true;
bool IsSendThreadRunning = true;

unsigned WINAPI RecvThread(void* Argument)
{
	SOCKET ServerSocket = *(SOCKET*)Argument;

	while (IsRecvThreadRunning)
	{
		PacketHeader Header;

		//header
		int RecvBytes = recv(ServerSocket, (char*)&Header, sizeof(Header), MSG_WAITALL);
		if (RecvBytes <= 0)
		{
			cout << "recv fail " << endl;
			break;
		}

		PacketType Type = static_cast<PacketType>(ntohs(Header.Type));
		unsigned short Size = ntohs(Header.Size);

		memset(RecvBuffer, 0, sizeof(RecvBuffer));
		//data JSON
		RecvBytes = recv(ServerSocket, RecvBuffer, Size, MSG_WAITALL);
		if (RecvBytes <= 0)
		{
			cout << "recv fail " << endl;
			break;
		}

		string JsonStr(RecvBuffer, RecvBytes);

		ChatPacket Data;
		switch (Type)
		{
		case PacketType::Position:
		{
			// 서버가 이동 결과를 Position 패킷으로 브로드캐스트
			PositionPacket Pos;
			Pos.Parse(JsonStr);
			cout << "[위치] " << Pos.UserID
				<< "( " << Pos.X << ", " << Pos.Y << " )" << "플레이어 아이콘: " << Pos.Shape << endl;
			break;
		}

		default:
			cout << "[클라] 알 수 없는 패킷 타입: "
				<< static_cast<int>(Type) << endl;
			break;
		}
	}

	return 0;
}

unsigned WINAPI SendThread(void* Argument)
{
	//책임은 사용하는 놈이 진다.
	SOCKET ServerSocket = *(SOCKET*)Argument;

	while (IsSendThreadRunning)
	{
		char c = _getch();

		if ((c == 'w' || c == 'a' || c == 's' || c == 'd')) 
		{
			MovePacket Move;
			Move.UserID = "Jihwan";
			Move.Dir = c;

			if (SendPacket(ServerSocket, PacketType::Move, Move.ToString()) <= 0)
			{
				cout << "[클라] MovePacket 전송 실패" << endl;
				break;
			}
		}
	}

	return 0;
}

int main()
{
	cout << "client" << endl;

	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ServerSockAddr;
	memset(&ServerSockAddr, 0, sizeof(ServerSockAddr));
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("192.168.0.59");
	ServerSockAddr.sin_port = htons(35000);

	connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	cout << "client connect" << endl;

	HANDLE ThreadHandles[2] = { 0, };

	//nonblocking, asynchrous
	ThreadHandles[0] = (HANDLE)_beginthreadex(0, 0, RecvThread, &ServerSocket, /*CREATE_SUSPENDED*/0, 0);
	ThreadHandles[1] = (HANDLE)_beginthreadex(0, 0, SendThread, &ServerSocket, /*CREATE_SUSPENDED*/0, 0);

	//blocking
	WaitForMultipleObjects(2, ThreadHandles, FALSE, INFINITE);

	closesocket(ServerSocket);

	IsSendThreadRunning = false;
	IsRecvThreadRunning = false;


	CloseHandle(ThreadHandles[0]);
	CloseHandle(ThreadHandles[1]);

	WSACleanup();

	return 0;
}