#pragma once
#include "TutorialGame.h"
#include "../CSC8503Common/GameServer.h"
#include "PacketReceiver.h"
#include "../CSC8503Common/PlayerObject.h"
#include "../CSC8503Common/StateMachine.h"

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

	void InitialiseAssets();
	void InitCamera();
	void InitWorld();


	GameObject* AddPlayerToWorld(int objID, const Vector3& position, float radius, float inverseMass = 10.0f);
	GameObject* AddWallToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddGoalToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddMovingToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddRobotToWorld(int objID, const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
	GameObject* AddSpinnerToWorld(int objID, const Vector3&position, Vector3 dimensions, float inverseMass = 10.0f, float spinVal = 10.0f);
	GameObject* AddFloorToWorld(int objID, const Vector3 & position, Vector3 dimensions);

protected:


	GameServer* server;
	StringPacketReceiver stringReceiver;
	PlayerConnectedPacketReceiver playerReceiver;
	ScorePacketReceiver scoreReceiver;
	BallForcePacketReceiver ballForceReceiver;
	ReadyPlayerPacketReceiver readyPlayerReceiver;
	
	//Player
	vector<int> playerIDs;
	std::map<int, string> players;
	std::map<int, int> scores;
	std::map<int, bool> readyPlayers;

	std::vector<GameObject*> dynamicObjects;
	std::vector<BallData*> ballData;
	std::map<int, PlayerObject*> playerBalls;
	std::map<int, bool> finishedLevel;

	GameWorld* world;
	PhysicsSystem* physics;
	GameTechRenderer* renderer;

	bool newPlayer = false;
	

	string level;

	Vector3 startingPos;

	int sendUpdate;

	vector<StateMachine*> stateMachines;

	int stateID;


	NCL::OGLMesh*	cubeMesh = nullptr;
	NCL::OGLMesh*	sphereMesh = nullptr;
	NCL::OGLTexture* basicTex = nullptr;
	NCL::OGLShader*	basicShader = nullptr;

	Level* lvl;
	bool finished;

};
