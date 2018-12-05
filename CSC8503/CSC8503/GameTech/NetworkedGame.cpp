#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#define COLLISION_MSG 30

NetworkGame::NetworkGame()
{
	NetworkBase::Initialise();
	int port = NetworkBase::GetDefaultPort();
	server = new GameServer(port, 2);
	serverReceiver = PacketReceivers("Server");

	server->RegisterPacketHandler(String, &serverReceiver);
}

NetworkGame::~NetworkGame()
{
	NetworkBase::Destroy();
}

void NetworkGame::UpdateServer()
{
	server->SendGlobalMessage(StringPacket("Server Side of the game is working!!"));
	server->UpdateServer();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

