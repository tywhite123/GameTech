#include "GameClient.h"
#include <iostream>
#include <string>

using namespace NCL;
using namespace CSC8503;

GameClient::GameClient()	{
	netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
}

GameClient::~GameClient()	{
	threadAlive = false;
	updateThread.join();
	enet_host_destroy(netHandle);
}

bool GameClient::Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum) {
	//Set up the address
	ENetAddress address;
	address.port = portNum; //Port
	address.host = (d << 24) | (c << 16) | (b << 8) | (a); //IP address to connect to

	//Connect to the server
	netPeer = enet_host_connect(netHandle, &address, 2, 0);
	return netPeer != nullptr;
}

void GameClient::UpdateClient() {
	if (netHandle == nullptr)
		return;

	ENetEvent event;

	
	while (enet_host_service(netHandle, &event, 0) > 0)
	{
		if (event.type == ENET_EVENT_TYPE_CONNECT) {
			*peerID = netPeer->outgoingPeerID;
			SendPacket(PlayerNamePacket(name));
			std::cout << "Connected to the server!" << std::endl;
		}
		else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
			std::cout << "Client: Packet Recived ..." << std::endl;
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet);
		}
		enet_packet_destroy(event.packet);
	}
}

void GameClient::SendPacket(GamePacket&  payload) {
	//Create the data packet and then send it
	ENetPacket* dataPacket = enet_packet_create(&payload, payload.GetTotalSize(), 0);
	enet_peer_send(netPeer, 0, dataPacket);
}

void GameClient::ThreadedUpdate() {
	while (threadAlive) {
		UpdateClient();
		std::this_thread::yield();
	}
}
