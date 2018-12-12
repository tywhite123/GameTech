#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"
#include <regex>
#include "RobotObject.h"
#include "MovingWallObject.h"
#include "../CSC8503Common/PlayerObject.h"
#include "SpinnerObject.h"
#include "../../Common/TextureLoader.h"

#define COLLISION_MSG 30
#define NAME "Server"

using namespace NCL;

NetworkGame::NetworkGame()
{
	world = new GameWorld();
	physics = new PhysicsSystem(*world);
	renderer = new GameTechRenderer(*world);
	physics->UseGravity(true);
	//players = 0;
	NetworkBase::Initialise();
	int port = NetworkBase::GetDefaultPort();
	server = new GameServer(port, 2);
	stringReceiver = StringPacketReceiver(NAME);
	playerReceiver = PlayerConnectedPacketReceiver(NAME, players);
	scoreReceiver = ScorePacketReceiver(NAME, scores);
	ballForceReceiver = BallForcePacketReceiver(NAME, ballData);
	readyPlayerReceiver = ReadyPlayerPacketReceiver(NAME, readyPlayers);

	stateID = 0;

	server->RegisterPacketHandler(Player_Connected, &playerReceiver);
	server->RegisterPacketHandler(String, &stringReceiver);
	server->RegisterPacketHandler(Score, &scoreReceiver);
	server->RegisterPacketHandler(Ball_Force, &ballForceReceiver);
	server->RegisterPacketHandler(Player_Ready, &readyPlayerReceiver);
	
	
	lvl = Level::GetInstance();
	InitialiseAssets();

	server->SetNewPlayer(newPlayer);	
	
	//LoadLevel("TestLevel1.txt");

	finished = false;

	sendUpdate = 0;
}

NetworkGame::~NetworkGame()
{
	delete physics;
	delete renderer;
	delete world;
	NetworkBase::Destroy();
}


void NetworkGame::InitialiseAssets()
{
	cubeMesh = new OGLMesh("cube.msh");
	cubeMesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	cubeMesh->UploadToGPU();

	sphereMesh = new OGLMesh("sphere.msh");
	sphereMesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	sphereMesh->UploadToGPU();

	basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	InitWorld();
}

void NetworkGame::InitCamera()
{
	world->GetMainCamera()->SetNearPlane(3.0f);
	world->GetMainCamera()->SetFarPlane(4200.0f);
	world->GetMainCamera()->SetPitch(-35.0f);
	world->GetMainCamera()->SetYaw(320.0f);
	world->GetMainCamera()->SetPosition(Vector3(-100, 120, 200));
}

void NetworkGame::InitWorld()
{

	world->ClearAndErase();
	physics->Clear();
	level = "";
	LoadLevel("TestLevel" + std::to_string(lvl->GetLevel()) + ".txt");
}


void NetworkGame::UpdateServer(float dt)
{
	
	world->GetMainCamera()->UpdateCamera(dt);
	world->UpdateWorld(dt);
	
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
	if (readyPlayers.size() == server->GetClientCount() && server->GetClientCount() != 0)
	{
		server->SendGlobalMessage(AllPlayersReadyPacket());
		readyPlayers.clear();
	}


	server->UpdateServer();


	for (StateMachine*s : stateMachines)
		s->Update();

	if (!finished) {
		for (auto ball : ballData)
		{
			std::cout << ball->objID << ": " << ball->ballForce << " " << ball->collidedAt << std::endl;
			playerBalls.at(ball->objID)->GetPhysicsObject()->AddForceAtPosition(ball->ballForce, ball->collidedAt);
			//playerBalls.at(ball->objID)->GetPhysicsObject()->ApplyLinearImpulse(ball->ballForce);
			delete ball;
		}
		ballData.clear();
	}

	renderer->Update(dt);
	physics->Update(dt);


	if (sendUpdate == 1 && !finished) {
		for (auto dObj : dynamicObjects) {
			server->SendGlobalMessage(ObjectDataPacket(stateID, dObj->GetObjID(),
				dObj->GetTransform().GetWorldPosition(), dObj->GetTransform().GetWorldOrientation(), dObj->GetPhysicsObject()->GetLinearVelocity()));

			stateID++;
		}

		sendUpdate = 0;

	}

	if(lvl->loadNext)
	{
		for(auto o : playerBalls)
		{
			if(o.second->GetObjID() == lvl->objID)
			{
				if (finishedLevel.count(o.first) < 1) {
					finishedLevel.insert(std::pair<int, bool>(o.first, lvl->loadNext));
					lvl->loadNext = false;
					lvl->SetLevel(lvl->GetLevel() - 1);
					server->SendGlobalMessage(LevelFinishedPacket(o.first));
				}
			}
		}
	}

	sendUpdate++;

	if (finishedLevel.size() == server->GetClientCount() && server->GetClientCount() != 0)
	{
		lvl->objID = -1;
		finished = true;
		lvl->loadNext = false;
		lvl->SetLevel(lvl->GetLevel() + 1);
		//wait for all players ready then load next level
		finishedLevel.clear();
		stateMachines.clear();
		dynamicObjects.clear();
		InitWorld();
		finished = false;
	}

	renderer->Render();

	//std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

	int objID = 0;

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
						AddWallToWorld(objID, pos, cubeDims, 0);
					}
					else if (in == 'S')
					{
						AddPlayerToWorld(objID, pos, cubeDims.x * 0.5f, 10.0f);
						startingPos = pos;

					}
					else if (in == 'E')
					{
						AddGoalToWorld(objID, pos, cubeDims*0.5f, 0);
					}
					else if (in == 'm')
					{
						AddMovingToWorld(objID, pos, Vector3(cubeDims.x*0.25f, cubeDims.y*0.35f, cubeDims.z), 1.f);

					}
					else if (in == 'r')
					{
						AddRobotToWorld(objID, pos, cubeDims*0.5f, 0);
					}
					else if (in == 's')
					{
						AddSpinnerToWorld(objID, pos, Vector3(cubeDims.x*0.15f, cubeDims.y*0.35f, cubeDims.z), 10.0f, 10.0f);
					}
					width++;
					objID++;
				}
				depth++;
				width = 0;
			}
			level += line + "\n";

		}
	}
	file.close();

	AddFloorToWorld(objID, Vector3((cubeDims.x*cubeDims.z) - cubeDims.x, -(cubeDims.y * 2)/*-cubeDims.y*/, (cubeDims.x*cubeDims.z) - cubeDims.z), Vector3((cubeDims.x*cubeDims.z), 10, (cubeDims.x*cubeDims.z)));

	std::cout << level << std::endl;
}

GameObject* NetworkGame::AddPlayerToWorld(int objID, const Vector3 & position, float radius, float inverseMass)
{
	PlayerObject* player = new PlayerObject(objID, "Player");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	player->SetBoundingVolume((CollisionVolume*)volume);
	player->GetTransform().SetWorldScale(sphereSize);
	player->GetTransform().SetWorldPosition(position);
	player->SetStartingPos(position);

	player->SetRenderObject(new RenderObject(&player->GetTransform(), sphereMesh, basicTex, basicShader));
	player->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), player->GetBoundingVolume()));

	player->GetPhysicsObject()->SetInverseMass(inverseMass);
	player->GetPhysicsObject()->InitSphereInertia();
	player->GetPhysicsObject()->SetElasticity(0.66f);
	player->GetPhysicsObject()->SetCanImpulse(true);
	player->GetPhysicsObject()->SetAffectedByGrav(true);
	player->level = lvl;

	playerBalls.insert(std::pair<int, PlayerObject*>(objID, player));

	dynamicObjects.push_back((GameObject*)player);
	world->AddGameObject((GameObject*)player);

	return player;
}

GameObject* NetworkGame::AddWallToWorld(int objID, const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	GameObject* wall = new GameObject(objID, "Wall");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	wall->SetBoundingVolume((CollisionVolume*)volume);

	wall->GetTransform().SetWorldPosition(position);
	wall->GetTransform().SetWorldScale(dimensions);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(inverseMass);
	wall->GetPhysicsObject()->InitCubeInertia();

	wall->GetPhysicsObject()->SetElasticity(1.66f);
	wall->GetPhysicsObject()->SetCanImpulse(false);

	world->AddGameObject(wall);

	return wall;
}

GameObject* NetworkGame::AddGoalToWorld(int objID, const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	GameObject* goal = new GameObject(objID, "Goal");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	goal->SetBoundingVolume((CollisionVolume*)volume);

	goal->GetTransform().SetWorldPosition(position);
	goal->GetTransform().SetWorldScale(dimensions);

	goal->SetRenderObject(new RenderObject(&goal->GetTransform(), cubeMesh, basicTex, basicShader));
	goal->SetPhysicsObject(new PhysicsObject(&goal->GetTransform(), goal->GetBoundingVolume()));

	goal->GetPhysicsObject()->SetInverseMass(inverseMass);
	goal->GetPhysicsObject()->InitCubeInertia();

	goal->GetPhysicsObject()->SetElasticity(1.66f);
	goal->GetPhysicsObject()->SetPhysical(false);

	world->AddGameObject(goal);

	return goal;
}

GameObject * NetworkGame::AddMovingToWorld(int objID, const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	MovingWallObject* movingWall = new MovingWallObject(objID, "Moving Wall");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	movingWall->SetBoundingVolume((CollisionVolume*)volume);

	movingWall->GetTransform().SetWorldPosition(position);
	movingWall->GetTransform().SetWorldScale(dimensions);

	movingWall->SetRenderObject(new RenderObject(&movingWall->GetTransform(), cubeMesh, basicTex, basicShader));
	movingWall->SetPhysicsObject(new PhysicsObject(&movingWall->GetTransform(), movingWall->GetBoundingVolume()));

	movingWall->GetPhysicsObject()->SetInverseMass(inverseMass);
	movingWall->GetPhysicsObject()->InitCubeInertia();

	movingWall->GetPhysicsObject()->SetElasticity(1.66f);
	movingWall->GetPhysicsObject()->SetPhysical(true);
	movingWall->GetPhysicsObject()->SetCanImpulse(false);
	movingWall->GetPhysicsObject()->SetAffectedByGrav(false);

	movingWall->SetupStateMachine();
	stateMachines.push_back(movingWall->GetStateMachine());

	dynamicObjects.push_back((GameObject*)movingWall);
	world->AddGameObject((GameObject*)movingWall);

	return movingWall;
}

GameObject * NetworkGame::AddRobotToWorld(int objID, const Vector3 & position, Vector3 dimensions, float inverseMass)
{
	RobotObject* robot = new RobotObject(objID, "Robot");

	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	robot->SetBoundingVolume((CollisionVolume*)volume);

	robot->GetTransform().SetWorldPosition(position);
	robot->GetTransform().SetWorldScale(dimensions);

	robot->SetRenderObject(new RenderObject(&robot->GetTransform(), cubeMesh, basicTex, basicShader));
	robot->SetPhysicsObject(new PhysicsObject(&robot->GetTransform(), robot->GetBoundingVolume()));

	robot->GetPhysicsObject()->SetInverseMass(inverseMass);
	robot->GetPhysicsObject()->InitCubeInertia();

	robot->GetPhysicsObject()->SetElasticity(0.f);
	robot->GetPhysicsObject()->SetPhysical(true);
	robot->GetPhysicsObject()->SetCanImpulse(false);
	robot->GetPhysicsObject()->SetAffectedByGrav(false);

	robot->SetupStateMachine();

	stateMachines.push_back(robot->GetStateMachine());


	dynamicObjects.push_back((GameObject*)robot);
	world->AddGameObject((GameObject*)robot);

	return robot;
}

GameObject* NetworkGame::AddSpinnerToWorld(int objID, const Vector3 & position, Vector3 dimensions, float inverseMass, float spinVal)
{
	SpinnerObject* spinner = new SpinnerObject(objID, "Spinner", spinVal);
	OBBVolume* volume = new OBBVolume(dimensions);

	spinner->SetBoundingVolume((CollisionVolume*)volume);

	spinner->GetTransform().SetWorldPosition(position);
	spinner->GetTransform().SetWorldScale(dimensions);

	spinner->SetRenderObject(new RenderObject(&spinner->GetTransform(), cubeMesh, basicTex, basicShader));
	spinner->SetPhysicsObject(new PhysicsObject(&spinner->GetTransform(), spinner->GetBoundingVolume()));

	spinner->GetPhysicsObject()->SetInverseMass(inverseMass);
	spinner->GetPhysicsObject()->InitCubeInertia();

	spinner->GetPhysicsObject()->SetElasticity(0.3f);
	spinner->GetPhysicsObject()->SetPhysical(true);
	spinner->GetPhysicsObject()->SetCanImpulse(false);
	spinner->GetPhysicsObject()->SetAffectedByGrav(false);

	spinner->SetupStateMachine();

	stateMachines.push_back(spinner->GetStateMachine());

	dynamicObjects.push_back((GameObject*)spinner);

	world->AddGameObject((GameObject*)spinner);

	return spinner;
}


//TODO: Update with the new one from singleplayer
GameObject* NetworkGame::AddFloorToWorld(int objID, const Vector3 & position, Vector3 dimensions)
{
	GameObject* floor = new GameObject(objID, "Floor");
	AABBVolume* volume = new AABBVolume(dimensions);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(dimensions);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	//floor->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	world->AddGameObject(floor);

	return floor;
}
