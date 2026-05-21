#include "pch.h"
#include "MovePacket.h"

//이동 방향 패킷

void MovePacket::Parse(std::string InString)
{
	JSONDocument.Parse(InString.c_str());

    UserID = JSONDocument["UserID"].GetString();
    Dir = static_cast<char>(JSONDocument["Dir"].GetInt());
}

std::string MovePacket::ToString()
{
    //JSONDocument를 문자열 변환 요청
    JSONDocument.SetObject();
    auto& Alloc = JSONDocument.GetAllocator();

    rapidjson::Value UID(UserID.c_str(), Alloc);
    JSONDocument.AddMember("UserID", UID, Alloc);
    JSONDocument.AddMember("Dir", Dir, Alloc);

    rapidjson::StringBuffer Buffer;
    rapidjson::Writer<rapidjson::StringBuffer> Writer(Buffer);
    JSONDocument.Accept(Writer);

    return Buffer.GetString();
}

int MovePacket::Length()
{
    return (int)ToString().length();
}

//--------------------------------------------------------------------------------------------------------------------------
//위치 패킷

void PositionPacket::Parse(std::string InString)
{
    JSONDocument.Parse(InString.c_str());
    UserID = JSONDocument["UserID"].GetString();
    X = JSONDocument["X"].GetInt();
    Y = JSONDocument["Y"].GetInt();
    Shape = static_cast<char>(JSONDocument["Shape"].GetInt());
}

std::string PositionPacket::ToString()
{
    JSONDocument.SetObject();
    auto& Alloc = JSONDocument.GetAllocator();

    rapidjson::Value UID(UserID.c_str(), Alloc);
    JSONDocument.AddMember("UserID", UID, Alloc);
    JSONDocument.AddMember("X", X, Alloc);
    JSONDocument.AddMember("Y", Y, Alloc);
    JSONDocument.AddMember("Shape", static_cast<int>(Shape), Alloc);

    rapidjson::StringBuffer Buf;
    rapidjson::Writer<rapidjson::StringBuffer> Writer(Buf);
    JSONDocument.Accept(Writer);
    return Buf.GetString();
}

int PositionPacket::Length()
{
    return static_cast<int>(ToString().length());
}
