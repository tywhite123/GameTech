#pragma once
#include "TutorialGame.h"
#include "../CSC8503Common/GameServer.h"
#include "PacketReceiver.h"

//TODO: Somehow sort this out
struct PlayerBall
{
	int playerToUpdate;
	Vector3 ballForce;
	Vector3 collidedAt;
};

class NetworkGame
{
public:
	NetworkGame();
	~NetworkGame();

	virtual void UpdateServer(float dt);

	void LoadLevel(string name);


	GameObject* AddPlayerToWorld(int objID, const Vector3& position, float radius, float inverseMass = 10.0f);
	GameObject* AddWallToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddGoalToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddMovingToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddRobotToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddFloorToWorld(int objID, const Vector3 & position, Vector3 dimensions);

protected:


	GameServer* server;
	StringPacketReceiver stringReceiver;
	PlayerConnectedPacketReceiver playerReceiver;
	ScorePacketReceiver scoreReceiver;
	BallForcePacketReceiver ballForceReceiver;
	
	//Player
	vector<int> playerIDs;
	std::map<int, string> players;
	std::map<int, int> scores;

	GameWorld* world;
	PhysicsSystem* physics;

	bool newPlayer = false;
	

	string level;

	Vector3 startingPos;
};
