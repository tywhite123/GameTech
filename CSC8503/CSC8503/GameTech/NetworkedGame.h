#pragma once
#include "TutorialGame.h"
#include "../CSC8503Common/GameServer.h"
#include "PacketReceiver.h"


class NetworkGame
{
public:
	NetworkGame();
	~NetworkGame();

	virtual void UpdateServer(float dt);

	void LoadLevel(string name);


	GameObject* AddPlayerToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
	GameObject* AddWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddGoalToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddMovingToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddRobotToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddFloorToWorld(const Vector3& position);

protected:


	GameServer* server;
	StringPacketReceiver stringReceiver;
	PlayerConnectedPacketReceiver playerReceiver;
	ScorePacketReceiver scoreReceiver;
	
	//Player
	vector<int> playerIDs;
	std::map<int, string> players;
	std::map<int, int> scores;

	GameWorld* world;
	PhysicsSystem* physics;

	bool newPlayer = false;
	

	string level;
};
