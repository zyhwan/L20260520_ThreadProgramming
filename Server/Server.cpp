#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "NetUtil.h"
#include "ChatPacket.h"
#include "MovePacket.h"

#include <winsock2.h>
#include <iostream>
#include <map>


#pragma comment(lib, "ws2_32")
#pragma comment(lib, "NetCommon")

using namespace std;

char Buffer[1024] = { 0, };

// 소켓별 플레이어 위치 저장
struct PlayerState
{
	string UserID;
	int X = 0;
	int Y = 0;
};
map<SOCKET, PlayerState> Players;

// 방향 -> 좌표 변환
static void ApplyDirection(char Dir, int& X, int& Y)
{
	switch (Dir)
	{
	case 'w': Y -= 1; break;
	case 's': Y += 1; break;
	case 'a': X -= 1; break;
	case 'd': X += 1; break;
	}
}

// 전체 브로드캐스트
static void Broadcast(fd_set& ReadSockets, SOCKET ListenSocket,
	PacketType Type, const string& JsonData)
{
	for (int j = 0; j < (int)ReadSockets.fd_count; ++j)
	{
		SOCKET Target = ReadSockets.fd_array[j];
		if (Target == ListenSocket)
			continue;

		if (SendPacket(Target, Type, JsonData) <= 0)
		{
			cout << "[해제] 브로드캐스트 실패" << endl;
			Players.erase(Target);
			DisconnectSocket(Target, &ReadSockets);
		}
	}
}


//blocking, synchrous, multiplexing(polling)
int main()
{
	cout << "server start" << endl;

	WSAData wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(35000);

	//already use port 이미 포트 사용중
	::bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, SOMAXCONN);



	//blocking, synchronous(TimeOut)
	TIMEVAL TimeOut;
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 500000;

	fd_set ReadSockets;
	fd_set CopyReadSockets;

	FD_ZERO(&ReadSockets);
	FD_SET(ListenSocket, &ReadSockets);

	while (true)
	{
		CopyReadSockets = ReadSockets;

		//0.5초씩 blocking
		int ChangeCount = select(0, &CopyReadSockets, 0, 0, &TimeOut);

		if (ChangeCount <= 0)
		{
			//Server Work
			//0.5초한번 서버 작업을 하는거
			continue;
		}

		//몬가 자료 있다.
		for (int i = 0; i < (int)ReadSockets.fd_count; ++i)
		{
			if (FD_ISSET(ReadSockets.fd_array[i], &CopyReadSockets))
			{
				if (ReadSockets.fd_array[i] == ListenSocket)
				{
					//connect process
					SOCKADDR_IN ClientSockAddr;
					memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
					int ClientSockSockLength = sizeof(ClientSockAddr);

					//blocking, synchronous
					SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockSockLength);

					cout << "connect client " << inet_ntoa(ClientSockAddr.sin_addr) << endl;

					FD_SET(ClientSocket, &ReadSockets);
					Players[ClientSocket] = PlayerState{};
				}
				else
				{
					//Data Receive

					//header
					PacketHeader Header;
					int RecvBytes = recv(ReadSockets.fd_array[i], (char*)&Header, HEADER_SIZE, MSG_WAITALL);
					if (RecvBytes <= 0)
					{
						cout << "header recv fail " << endl;
						Players.erase(ReadSockets.fd_array[i]);
						DisconnectSocket(ReadSockets.fd_array[i], &ReadSockets);
						continue;
					}

					PacketType Type = static_cast<PacketType>(ntohs(Header.Type));
					unsigned short DataSize = ntohs(Header.Size);

					memset(Buffer, 0, sizeof(Buffer));
					//data JSON
					RecvBytes = recv(ReadSockets.fd_array[i], Buffer, DataSize, MSG_WAITALL);
					if (RecvBytes <= 0)
					{
						cout << "data recv fail " << endl;
						Players.erase(ReadSockets.fd_array[i]);
						DisconnectSocket(ReadSockets.fd_array[i], &ReadSockets);
						continue;
					}

					string JsonStr(Buffer, RecvBytes);

					// 3) PacketType 별 처리
					switch (Type)
					{
						// ── Chat ───────────────────────────────
					case PacketType::Chat:
					{
						ChatPacket Chat;
						Chat.Parse(JsonStr);

						if (Players[ReadSockets.fd_array[i]].UserID.empty())
							Players[ReadSockets.fd_array[i]].UserID = Chat.UserID;

						cout << "[채팅] " << Chat.UserID
							<< " : " << Chat.Message
							<< " (Gold: " << Chat.Gold << ")" << endl;

						for (int j = 0; j < (int)ReadSockets.fd_count; ++j)
						{
							SOCKET Target = ReadSockets.fd_array[j];
							if (Target == ListenSocket)
								continue;

							if (SendPacket(Target, Type, Chat.ToString()) <= 0)
							{
								cout << "[해제] 브로드캐스트 실패" << endl;
								Players.erase(Target);
								DisconnectSocket(Target, &ReadSockets);
							}
						}
						break;
					}

					// ── Move ───────────────────────────────
					case PacketType::Move:
					{
						MovePacket Move;
						Move.Parse(JsonStr);

						PlayerState& State = Players[ReadSockets.fd_array[i]];
						State.UserID = Move.UserID;
						ApplyDirection(Move.Dir, State.X, State.Y);

						cout << "[이동] " << State.UserID
							<< " Dir=" << Move.Dir
							<< " -> (" << State.X << ", " << State.Y << ")" << endl;

						PositionPacket Pos;
						Pos.UserID = State.UserID;
						Pos.X = State.X;
						Pos.Y = State.Y;

						for (int j = 0; j < (int)ReadSockets.fd_count; ++j)
						{
							SOCKET Target = ReadSockets.fd_array[j];
							if (Target == ListenSocket)
								continue;

							if (SendPacket(Target, PacketType::Position, Pos.ToString()) <= 0)
							{
								cout << "[해제] 브로드캐스트 실패" << endl;
								Players.erase(Target);
								DisconnectSocket(Target, &ReadSockets);
							}
						}

						break;
					}

					default:
						cout << "[서버] 알 수 없는 패킷 타입" << endl;
						break;
					}
				}
			}
		}
	}


	closesocket(ListenSocket);
	WSACleanup();

	return 0;
}