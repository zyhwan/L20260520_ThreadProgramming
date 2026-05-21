#pragma once
#include "pch.h"

enum class PacketType
{
    Move = 0,   // 클라이언트 -> 서버: 이동 방향
    Position,   // 서버 -> 클라이언트: 플레이어 위치
    Max
};

struct PacketHeader
{
	unsigned short Type;
	unsigned short Size;
};

constexpr int HEADER_SIZE = sizeof(PacketHeader); // 4 bytes

class IPacket
{
public:
	virtual void Parse(std::string InString) = 0;
	virtual std::string ToString() = 0;
	virtual int Length() = 0;

	rapidjson::Document JSONDocument;
};