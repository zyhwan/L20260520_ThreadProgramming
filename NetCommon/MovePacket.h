#pragma once
#include "Packet.h"

class MovePacket : public IPacket
{
public:
	std::string UserID;
	char Dir;

	void Parse(std::string InString) override;
	std::string ToString() override;
	int Length() override;
};

class PositionPacket : public IPacket
{
public:
	std::string UserID;
	int         X = 0;
	int         Y = 0;

	void        Parse(std::string InString) override;
	std::string ToString()                  override;
	int         Length()                    override;
};