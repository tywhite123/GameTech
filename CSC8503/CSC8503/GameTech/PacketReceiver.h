#pragma once
#include "../CSC8503Common/NetworkBase.h"
#include <iostream>



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





