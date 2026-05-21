#include "pch.h"

#include "NetUtil.h"

int SendAll(SOCKET ReceiverSocket, const char* Data, int Size)
{
	int TotalSendDataSize = 0;
	int WantSendDataSize = Size;
	int SentBytes = 0;
	int Count = 0;
	do
	{
		SentBytes = send(ReceiverSocket, Data + TotalSendDataSize, WantSendDataSize - TotalSendDataSize, 0);
		TotalSendDataSize += SentBytes;
		if (SentBytes <= 0)
		{
			return SentBytes;
		}
	} while (TotalSendDataSize < WantSendDataSize);

	return WantSendDataSize;
}

void DisconnectSocket(SOCKET DisconnectedSocket, fd_set* Sockets)
{
	SOCKADDR_IN ClosedSockAddr;
	memset(&ClosedSockAddr, 0, sizeof(ClosedSockAddr));
	int ClosedSockAddrLength = sizeof(ClosedSockAddr);

	SOCKET ClosedSocket = DisconnectedSocket;
	getpeername(ClosedSocket, (SOCKADDR*)&ClosedSockAddr, &ClosedSockAddrLength);
	FD_CLR(DisconnectedSocket, &Sockets);
	closesocket(ClosedSocket);
}

int SendPacket(SOCKET ReceiverSocket, PacketType Type, const std::string& JsonData)
{
	PacketHeader Header;
	Header.Type = htons(static_cast<unsigned short>(Type));
	Header.Size = htons(static_cast<unsigned short>(JsonData.length()));

	// 헤더 전송
	int Sent = SendAll(ReceiverSocket, reinterpret_cast<const char*>(&Header), HEADER_SIZE);
	if (Sent <= 0)
		return Sent;

	// 본문(JSON) 전송
	Sent = SendAll(ReceiverSocket, JsonData.c_str(), static_cast<int>(JsonData.length()));
	return Sent;
}
