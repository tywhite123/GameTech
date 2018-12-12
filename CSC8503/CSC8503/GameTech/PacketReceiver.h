#pragma once
#include "../CSC8503Common/NetworkBase.h"
#include <iostream>

struct UpdateData
{
	int stateID;
	int objID;
	Vector3 pos;
	Quaternion ori;
	Vector3 vel;

	UpdateData(int sID, int objID, Vector3 pos, Quaternion ori, Vector3 vel)
	{
		stateID = sID;
		this->objID = objID;
		this->pos = pos;
		this->ori = ori;
		this->vel = vel;
	}
};

struct BallData
{
	int peerID;
	int objID;
	Vector3 ballForce;
	Vector3 collidedAt;

	BallData(int peerID, int objID, Vector3 ballForce, Vector3 collidedAt)
	{
		this->peerID = peerID;
		this->objID = objID;
		this->ballForce = ballForce;
		this->collidedAt = collidedAt;
	}
};

class StringPacketReceiver : public PacketReceiver
{
public:
	StringPacketReceiver() {};
	StringPacketReceiver(std::string name)
	{
		this->name = name;
	}

	std::string GetName() { return name; }

	void ReceivePacket(int type, GamePacket* payload, int source) override
	{
		if (type == String)
		{
			StringPacket* realPacket = (StringPacket*)payload;
			std::string msg = realPacket->GetStringFromData();
			std::cout << source << " received message: " << msg << std::endl;
		}

	}

protected:
	std::string name;
};


class ScorePacketReceiver : public PacketReceiver
{
public:
	ScorePacketReceiver() {};
	ScorePacketReceiver(std::string name, std::map<int, int>& score)
	{
		this->name = name;
		scores = &score;
	}

	std::string GetName() { return name; }

	void ReceivePacket(int type, GamePacket* payload, int source) override
	{
		if (type == Score)
		{
			ScorePacket* realPacket = (ScorePacket*)payload;
			int score = realPacket->score;
			scores->insert(std::pair<int, int>(source, score));
			std::cout << name << " Score " << source << ": " << score << std::endl;
		}

	}

protected:
	std::string name;
	std::map<int, int>* scores;
};

class PlayerConnectedPacketReceiver:public PacketReceiver
{
public:
	PlayerConnectedPacketReceiver() {};
	PlayerConnectedPacketReceiver(std::string name, std::map<int, string>& p)
	{
		this->name = name;
		players = &p;
	}

	std::string GetName() { return name; }


	void ReceivePacket(int type, GamePacket* payload, int source) override
	{
		if (type == Player_Connected)
		{
			StringPacket* realPacket = (StringPacket*)payload;
			std::string uName = realPacket->GetStringFromData();
			players->insert(std::pair<int, string>(source, uName));
			std::cout << players->at(source) << std::endl;
			
		}
	}

protected:
	std::string name;
	std::map<int, string>* players;
};


class LevelPacketReceiver : public PacketReceiver
{
public:
	LevelPacketReceiver() {};
	LevelPacketReceiver(std::string name, std::string& l)
	{
		this->name = name;
		level = &l;
	}
	std::string GetName() { return name; }


	void ReceivePacket(int type, GamePacket* payload, int source) override
	{
		if (type == New_Level)
		{
			StringPacket* realPacket = (StringPacket*)payload;
			*level = realPacket->GetStringFromData();
			std::cout << *level << std::endl;

		}
	}

protected:
	std::string name;
	std::string* level;
};


class ObjectPacketReceiver : public PacketReceiver
{
public:
	ObjectPacketReceiver() {};
	//TODO: OBJECT CONSTRUCTOR
	ObjectPacketReceiver(string name, vector<UpdateData*>& data)
	{
		this->name = name;
		updateData = &data;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) override
	{
		if (type == Object_Data)
		{
			ObjectDataPacket* data = (ObjectDataPacket*)payload;
			UpdateData* d = new UpdateData(data->stateID, data->objID, data->pos, data->ori, data->vel);
			updateData->push_back(d);
		}
	}
	
protected:
	string name;
	vector<UpdateData*>* updateData;
};

class BallForcePacketReceiver : public PacketReceiver
{
public:
	BallForcePacketReceiver() {};
	BallForcePacketReceiver(string name, std::vector<BallData*>& ballData)
	{
		this->name = name;
		this->ballData = &ballData;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) override
	{
		if(type == Ball_Force)
		{
			BallForcePacket* data = (BallForcePacket*)payload;
			int peerID = source;
			int objID = data->objID;
			Vector3 ballForce = data->ballForce;
			Vector3 collidedAt = data->collidedAt;
			BallData* b = new BallData(peerID, objID/*should be objID*/, ballForce, collidedAt); //TODO: change to objID
			ballData->push_back(b); //TODO: maybe change?!
		}
	}

protected:
	string name;
	std::vector<BallData*>* ballData;
};


class ReadyPlayerPacketReceiver : public PacketReceiver
{
public:
	ReadyPlayerPacketReceiver(){}
	ReadyPlayerPacketReceiver(string name, std::map<int, bool>& readyPlayers){
		this->name = name;
		this->readyPlayers = &readyPlayers;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) override
	{
		if (type == Player_Ready) //Ready
		{
			ReadyPlayerPacket* data = (ReadyPlayerPacket*)payload;
			int peerID = source;
			bool ready = data->ready;
			readyPlayers->insert(std::pair<int, bool>(peerID, ready));
		}
	}

protected:
	string name;
	std::map<int, bool>* readyPlayers;
};

class AllPlayersReadyReceiver:public PacketReceiver
{
public:
	AllPlayersReadyReceiver(){}
	AllPlayersReadyReceiver(string name, bool& ready)
	{
		this->name = name;
		this->ready = &ready;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) override
	{
		if (type == All_Players_Ready) //Ready
		{
			AllPlayersReadyPacket* data = (AllPlayersReadyPacket*)payload;
			*ready = data->allReady;
			
		}
	}

protected:
	string name;
	bool* ready;
};


class LevelFinishedPacketReceiver : public PacketReceiver
{
public:
	LevelFinishedPacketReceiver() {}
	LevelFinishedPacketReceiver(string name, bool& fin)
	{
		this->name = name;
		this->fin = &fin;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) override
	{
		if (type == Level_Finished) //Ready
		{
			LevelFinishedPacket* data = (LevelFinishedPacket*)payload;
			*fin = data->finished;

		}
	}

protected:
	string name;
	bool* fin;
};





