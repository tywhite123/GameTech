#pragma once
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"
#include "NetworkedGame.h"
#include "GolfGame.h"

using namespace NCL::CSC8503;

struct GameStates
{
	GameServer* server = nullptr;
	GameClient* client = nullptr;
	NetworkGame* netGame = nullptr;
	GolfGame* golf = nullptr;

};
