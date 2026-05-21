#pragma once

#include "pch.h"
#include "Packet.h"

extern int SendAll(SOCKET ReceiverSocket, const char* Data, int Size);

extern void DisconnectSocket(SOCKET DisconnectedSocket, fd_set* Sockets);

extern int SendPacket(SOCKET ReceiverSocket, PacketType Type, const std::string& JsonData);