#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"
#include <regex>
#include "RobotObject.h"
#include "MovingWallObject.h"
#include "../CSC8503Common/PlayerObject.h"

#define COLLISION_MSG 30
#define NAME "Server"

using namespace NCL;

NetworkGame::NetworkGame()
{
	world = new GameWorld();
	physics = new PhysicsSystem(*world);
	//players = 0;
	NetworkBase::Initialise();
	int port = NetworkBase::GetDefaultPort();
	server = new GameServer(port, 2);
	stringReceiver = StringPacketReceiver(NAME);
	playerReceiver = PlayerConnectedPacketReceiver(NAME, players);
	scoreReceiver = ScorePacketReceiver(NAME, scores);
	//ballForceReceiver = BallForcePacketReceiver(NAME, playerToUpdate, ballForce, collidedAt);


	server->RegisterPacketHandler(Player_Connected, &playerReceiver);
	server->RegisterPacketHandler(String, &stringReceiver);
	server->RegisterPacketHandler(Score, &scoreReceiver);
	//server->RegisterPacketHandler(Ball_Force, &ballForceReceiver);

	server->SetNewPlayer(newPlayer);
	LoadLevel("TestLevel1.txt");
}

NetworkGame::~NetworkGame()
{
	delete physics;
	delete world;
	NetworkBase::Destroy();
}

void NetworkGame::UpdateServer(float dt)
{
	world->UpdateWorld(dt);
	physics->Update(dt);
	//server->SendGlobalMessage(StringPacket("Server Side of the game is working!!"));
	if(newPlayer)
	{
		server->SendGlobalMessage(StringPacket(level));
		newPlayer = false;
	}
	if (scores.size() == server->GetClientCount() && server->GetClientCount() != 0)
	{
		string toSend = "Scores: \n";
		std::cout << "Scores: " << std::endl;
		for(int i = 0; i < scores.size(); ++i)
		{
			toSend += players.at(i) + ": " + std::to_string(scores.at(i)) + "\n";
			std::cout << players.at(i) << ": " << scores.at(i) << std::endl;
		}
		server->SendGlobalMessage(StringPacket(toSend));
		scores.clear();
	}

	server->UpdateServer();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

/**
*	Level File Format Key:
*	S - Ball Start
*	E - End/Goal
*	x - Wall Cube
*	. - Empty Space
*	m - Moving Wall
*/
void NetworkGame::LoadLevel(std::string filename)
{
	std::ifstream file("../../Assets/Data/" + filename);

	int width = 0;
	int depth = 0;
	int x, y, z;
	Vector3 cubeDims;
	int i = 0;

	if (file.is_open())
	{
		string line;
		while (getline(file, line)) {
			if (std::regex_match(line, std::regex("[0-9]+")))
			{
				switch (i)
				{
				case 0: x = stoi(line); break;
				case 1: y = stoi(line); break;
				case 2:
					z = stoi(line);
					cubeDims = Vector3(x, y, z);
					break;
				}
				i++;
			}
			else
			{
				for (char& in : line) {
					Vector3 pos(width*(cubeDims.x * 2), 0, depth*(cubeDims.z * 2));
					if (in == 'x')
					{
						AddWallToWorld(pos, cubeDims, 0);
					}
					else if (in == 'S')
					{
						AddPlayerToWorld(pos, cubeDims.x * 0.5f, 10.0f);

					}
					else if (in == 'E')
					{
						AddGoalToWorld(pos, cubeDims*0.5f, 0);
					}
					else if (in == 'm')
					{
						AddMovingToWorld(pos, Vector3(cubeDims.x*0.25f, cubeDims.y*0.35f, cubeDims.z), 1.f);

					}
					else if (in == 'r')
					{
						AddRobotToWorld(pos, cubeDims*0.5f, 0);
					}
					width++;
				}
				depth++;
				width = 0;
			}
			level += line + "\n";

		}
	}
	file.close();

	std::cout << level << std::endl;
}

GameObject* NetworkGame::AddPlayerToWorld(const Vector3 & position, float radius, float inverseMass)
{
	PlayerObject* player = new PlayerObject("Player");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	player->SetBoundingVolume((CollisionVolume*)volume);
	player->GetTransform().SetWorldScale(sphereSize);
	player->GetTransform().SetWorldPosition(position);
	player->SetStartingPos(position);

	//player->SetRenderObject(new RenderObject(&player->GetTransform(), sphereMesh, basicTex, basicShader));
	player->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), player->GetBoundingVolume()));

	player->GetPhysicsObject()->SetInverseMass(inverseMass);
	player->GetPhysicsObject()->InitSphereInertia();
	player->GetPhysicsObject()->SetElasticity(0.66f);
	player->GetPhysicsObject()->SetCanImpulse(true);
	player->GetPhysicsObject()->SetAffectedByGrav(true);
	//player->level = level;

	world->AddGameObject((GameObject*)player);

	return player;
}

GameObject* NetworkGame::AddWallToWorld(const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	GameObject* wall = new GameObject("Wall");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	wall->SetBoundingVolume((CollisionVolume*)volume);

	wall->GetTransform().SetWorldPosition(position);
	wall->GetTransform().SetWorldScale(dimensions);

	//wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(inverseMass);
	wall->GetPhysicsObject()->InitCubeInertia();

	wall->GetPhysicsObject()->SetElasticity(1.66f);
	wall->GetPhysicsObject()->SetCanImpulse(false);

	world->AddGameObject(wall);

	return wall;
}

GameObject* NetworkGame::AddGoalToWorld(const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	GameObject* goal = new GameObject("Goal");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	goal->SetBoundingVolume((CollisionVolume*)volume);

	goal->GetTransform().SetWorldPosition(position);
	goal->GetTransform().SetWorldScale(dimensions);

	//goal->SetRenderObject(new RenderObject(&goal->GetTransform(), cubeMesh, basicTex, basicShader));
	goal->SetPhysicsObject(new PhysicsObject(&goal->GetTransform(), goal->GetBoundingVolume()));

	goal->GetPhysicsObject()->SetInverseMass(inverseMass);
	goal->GetPhysicsObject()->InitCubeInertia();

	goal->GetPhysicsObject()->SetElasticity(1.66f);
	goal->GetPhysicsObject()->SetPhysical(false);

	world->AddGameObject(goal);

	return goal;
}

GameObject * NetworkGame::AddMovingToWorld(const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	MovingWallObject* movingWall = new MovingWallObject("Moving Wall");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	movingWall->SetBoundingVolume((CollisionVolume*)volume);

	movingWall->GetTransform().SetWorldPosition(position);
	movingWall->GetTransform().SetWorldScale(dimensions);

	//movingWall->SetRenderObject(new RenderObject(&movingWall->GetTransform(), cubeMesh, basicTex, basicShader));
	movingWall->SetPhysicsObject(new PhysicsObject(&movingWall->GetTransform(), movingWall->GetBoundingVolume()));

	movingWall->GetPhysicsObject()->SetInverseMass(inverseMass);
	movingWall->GetPhysicsObject()->InitCubeInertia();

	movingWall->GetPhysicsObject()->SetElasticity(1.66f);
	movingWall->GetPhysicsObject()->SetPhysical(true);
	movingWall->GetPhysicsObject()->SetCanImpulse(false);
	movingWall->GetPhysicsObject()->SetAffectedByGrav(false);

	movingWall->SetupStateMachine();

	//stateMachines.push_back(movingWall->GetStateMachine());

	world->AddGameObject((GameObject*)movingWall);

	return movingWall;
}

GameObject * NetworkGame::AddRobotToWorld(const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	RobotObject* robot = new RobotObject("Robot");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	robot->SetBoundingVolume((CollisionVolume*)volume);

	robot->GetTransform().SetWorldPosition(position);
	robot->GetTransform().SetWorldScale(dimensions);

	//robot->SetRenderObject(new RenderObject(&robot->GetTransform(), cubeMesh, basicTex, basicShader));
	robot->SetPhysicsObject(new PhysicsObject(&robot->GetTransform(), robot->GetBoundingVolume()));

	robot->GetPhysicsObject()->SetInverseMass(inverseMass);
	robot->GetPhysicsObject()->InitCubeInertia();

	robot->GetPhysicsObject()->SetElasticity(0.f);
	robot->GetPhysicsObject()->SetPhysical(true);
	robot->GetPhysicsObject()->SetCanImpulse(false);
	robot->GetPhysicsObject()->SetAffectedByGrav(false);

	robot->SetupStateMachine();

	//stateMachines.push_back(robot->GetStateMachine());

	world->AddGameObject((GameObject*)robot);

	return robot;
}

GameObject* NetworkGame::AddFloorToWorld(const Vector3 & position)
{
	GameObject* floor = new GameObject("Floor");

	Vector3 floorSize = Vector3(1000, 10, 1000);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(floorSize);
	floor->GetTransform().SetWorldPosition(position);

	//floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}
