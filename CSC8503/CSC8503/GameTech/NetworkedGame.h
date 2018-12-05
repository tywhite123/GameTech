#pragma once
#include "TutorialGame.h"
#include "../CSC8503Common/GameServer.h"
#include "PacketReceivers.h"


class NetworkGame
{
public:
	NetworkGame();
	~NetworkGame();

	virtual void UpdateServer();



protected:


	GameServer* server;
	PacketReceivers serverReceiver;
	vector<int> playerIDs;
	

};
