#pragma once
#include "../CSC8503Common/NetworkBase.h"
#include <iostream>

class PacketReceivers : public PacketReceiver
{
public:
	PacketReceivers() {};
	PacketReceivers(std::string name)
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
			std::cout << name << " received message: " << msg << std::endl;
		}
	}

protected:
	std::string name;
};
