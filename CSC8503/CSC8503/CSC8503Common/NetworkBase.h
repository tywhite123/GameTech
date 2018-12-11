#pragma once
#include <enet/enet.h>
#include <map>
#include <string>
#include "../../Common/Vector3.h"
#include "../../Common/Quaternion.h"
#include "CollisionVolume.h"

using namespace NCL::Maths;

enum BasicNetworkMessages {
	None,
	Hello,
	Message,
	String,
	Delta_State,	//1 byte per channel since the last state
	Full_State,		//Full transform etc
	Received_State, //received from a client, informs that its received packet n
	Player_Connected,
	Player_Disconnected,
	Shutdown,
	Score,
	New_Level,
	Ball_Force,
	Object_Data,
	Player_Ready,
	All_Players_Ready
};

struct GamePacket {
	short size;
	short type;

	GamePacket() {
		type		= BasicNetworkMessages::None;
		size		= 0;
	}

	GamePacket(short type) {
		this->type	= type;
	}

	int GetTotalSize() {
		return sizeof(GamePacket) + size;
	}
};

struct StringPacket : public GamePacket {
	char	stringData[256];

	StringPacket(const std::string& message) {
		type		= BasicNetworkMessages::String;
		size		= (short)message.length();

		memcpy(stringData, message.data(), size);
	};

	std::string GetStringFromData() {
		std::string realString(stringData);
		realString.resize(size);
		return realString;
	}
};

struct PlayerNamePacket : public GamePacket {
	char	stringData[256];

	PlayerNamePacket(const std::string& message) {
		type = BasicNetworkMessages::Player_Connected;
		size = (short)message.length();

		memcpy(stringData, message.data(), size);
	};

	std::string GetStringFromData() {
		std::string realString(stringData);
		realString.resize(size);
		return realString;
	}
};

struct NewPlayerPacket : public GamePacket {
	int playerID;
	NewPlayerPacket(int p ) {
		type		= BasicNetworkMessages::Player_Connected;
		playerID	= p;
		size		= sizeof(int);
	}
};

struct PlayerDisconnectPacket : public GamePacket {
	int playerID;
	PlayerDisconnectPacket(int p) {
		type		= BasicNetworkMessages::Player_Disconnected;
		playerID	= p;
		size		= sizeof(int);
	}
};

struct ScorePacket : public GamePacket {
	int score;
	ScorePacket(int s) {
		type = BasicNetworkMessages::Score;
		score = s;
		size = sizeof(int);
	}
};


struct LevelPacket : public GamePacket
{
	char	stringData[256];

	LevelPacket(const std::string& message) {
		type = BasicNetworkMessages::New_Level;
		size = (short)message.length();

		memcpy(stringData, message.data(), size);
	};

	std::string GetStringFromData() {
		std::string realString(stringData);
		realString.resize(size);
		return realString;
	}
};

struct BallForcePacket : public GamePacket
{
	int objID;
	Vector3 ballForce;
	Vector3 collidedAt;

	BallForcePacket(int objID, Vector3 force, Vector3 collided)
	{
		this->objID = objID;
		ballForce = force;
		collidedAt = collided;
		size = (sizeof(Vector3) * 2);
		type = BasicNetworkMessages::Ball_Force;
	}

};

struct ObjectDataPacket : public GamePacket
{
	int objID;
	Vector3 pos;
	Quaternion ori;
	
	ObjectDataPacket(int objID, Vector3 pos, Quaternion ori)
	{
		this->objID = objID;
		this->pos = pos;
		this->ori = ori;
		size = (sizeof(int)) + sizeof(Vector3) + sizeof(Quaternion);
		type = Object_Data;
	}
	
};

struct ReadyPlayerPacket : public GamePacket
{
	bool ready;

	ReadyPlayerPacket(bool r = true)
	{
		ready = r;
		size = sizeof(bool);
		type = Player_Ready;
	}
};

struct AllPlayersReadyPacket : public GamePacket
{
	bool allReady;

	AllPlayersReadyPacket(bool r = true)
	{
		allReady = r;
		size = sizeof(bool);
		type = All_Players_Ready;
	}
};


class PacketReceiver {
public:
	virtual void ReceivePacket(int type, GamePacket* payload, int source = -1) = 0;
};

class NetworkBase	{
public:
	static void Initialise();
	static void Destroy();

	static int GetDefaultPort() {
		return 1234;
	}

	void RegisterPacketHandler(int msgID, PacketReceiver* receiver) {
		packetHandlers.insert(std::make_pair(msgID, receiver));
	}

protected:
	NetworkBase();
	~NetworkBase();

	bool ProcessPacket(GamePacket* packet, int peerID = -1);

	typedef std::multimap<int, PacketReceiver*>::const_iterator PacketHandlerIterator;

	bool GetPacketHandlers(int msgID, PacketHandlerIterator& first, PacketHandlerIterator& last) const {
		auto range = packetHandlers.equal_range(msgID);

		if (range.first == packetHandlers.end()) {
			return false; //no handlers for this message type!
		}
		first	= range.first;
		last	= range.second;
		return true;
	}

	ENetHost* netHandle;

	std::multimap<int, PacketReceiver*> packetHandlers;
};